cmake_minimum_required(VERSION 2.6)

project(ImageCompleter)

########### Eigen ##############
include_directories(/media/portable/src/Eigen /media/portable/src/Eigen/unsupported)

set(CMAKE_MODULE_PATH /media/portable/src/Eigen/cmake)

FIND_PACKAGE(Umfpack REQUIRED)
FIND_PACKAGE(BLAS REQUIRED)

include_directories(${UMFPACK_INCLUDES})

list(APPEND CORE_DEFINITIONS "EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")
# must link to umfpack

################################

find_package(wxWidgets COMPONENTS core base REQUIRED)
include( ${wxWidgets_USE_FILE} )

set(cmdsrc src/image-completer-cmd)
set(ImageCompleterDir src/image-completer-lib/impl)
set(tech src/tech)

include_directories(src/image-completer-cmd src/image-completer-lib src/image-completer-lib/api src/image-completer-lib/impl src/image-completer-lib/impl/compositors src/image-completer-lib/impl/energy-calculators src/tech/tech src/tech)

#If you want to enable the patches to be composited using Poisson blending, set this flag to ON (requires TAUCS to be installed).
set(USE_POISSON OFF CACHE BOOL "Use Poisson blending?")
if(USE_POISSON)
  #set(MAIN_BUILD_FLAGS "${MAIN_BUILD_FLAGS} -DUSE_POISSON")
  set(MAIN_BUILD_DEFINITIONS "${MAIN_BUILD_DEFINITIONS} USE_POISSON;")
  set(LibrariesToLink ${LibrariesToLink} umfpack)
endif(USE_POISSON)

#If you want to enable the per-pixel energy calculator to use multiple hardware threads, set this flag to ON.
set(USE_THREADS OFF CACHE BOOL "Use threads?" FORCE)
if(USE_THREADS)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_THREADS")
endif(USE_THREADS)

IF(UNIX)
  list(APPEND CORE_DEFINITIONS "UNIX")
ENDIF(UNIX)

set(USE_ITK OFF CACHE BOOL "Use ITK?")
IF(USE_ITK)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_ITK")
ENDIF(USE_ITK)

set(USE_FLOAT_PIXELS OFF CACHE BOOL "Use float pixels?")
IF(USE_FLOAT_PIXELS)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_FLOAT_PIXELS")
ENDIF(USE_FLOAT_PIXELS)

set(PIXEL_DIMENSION 3 CACHE STRING "Dimensionality of pixels?") #cmake doesn't have an "INT" variable type, so treat it as a string
list(APPEND MAIN_BUILD_DEFINITIONS "PIXEL_DIMENSION=${PIXEL_DIMENSION}")

###############
#if Eigen is setup correctly, we would want to enable Poisson blending
IF(USE_POISSON)
  set(ImageCompleterSources ${ImageCompleterSources}
  ${ImageCompleterDir}/compositors/PatchTypePoisson.cpp
  ${ImageCompleterDir}/compositors/PoissonSolver.cpp)
ENDIF(USE_POISSON)

set(ImageCompleterSources ${ImageCompleterSources}

${cmdsrc}/AppData.cpp

${ImageCompleterDir}/Compositor.cpp
${ImageCompleterDir}/ConstNodeLabels.cpp
${ImageCompleterDir}/EnergyCalculatorContainer.cpp
${ImageCompleterDir}/ImageConst.cpp
${ImageCompleterDir}/ImageScalable.cpp
${ImageCompleterDir}/Label.cpp
${ImageCompleterDir}/LfnIc.cpp
${ImageCompleterDir}/LfnIcSettings.cpp
${ImageCompleterDir}/MaskScalable.cpp
${ImageCompleterDir}/MaskWritable.cpp
${ImageCompleterDir}/NeighborEdge.cpp
${ImageCompleterDir}/Node.cpp
${ImageCompleterDir}/NodeSet.cpp
${ImageCompleterDir}/Pch.cpp
${ImageCompleterDir}/PriorityBpRunner.cpp
${ImageCompleterDir}/ScalableDebugging.cpp
${ImageCompleterDir}/SettingsScalable.cpp

${ImageCompleterDir}/compositors/CompositorRoot.cpp
${ImageCompleterDir}/compositors/CompositorUtils.cpp
${ImageCompleterDir}/compositors/ImageFloat.cpp
${ImageCompleterDir}/compositors/OutputBlenderDebugSoftMaskIntensity.cpp
${ImageCompleterDir}/compositors/OutputBlenderSoftMask.cpp
${ImageCompleterDir}/compositors/OutputBlenderNone.cpp
${ImageCompleterDir}/compositors/PatchBlenderNone.cpp
${ImageCompleterDir}/compositors/PatchBlenderPriority.cpp
${ImageCompleterDir}/compositors/PatchTypeDebugPatchOrder.cpp
${ImageCompleterDir}/compositors/PatchTypeNormal.cpp

${ImageCompleterDir}/energy-calculators/EnergyCalculatorFft.cpp
${ImageCompleterDir}/energy-calculators/EnergyCalculatorFftUtils.cpp
${ImageCompleterDir}/energy-calculators/EnergyCalculatorPerPixel.cpp

)

add_library(ImageCompleterLib ${ImageCompleterSources})
set_target_properties(ImageCompleterLib PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${MAIN_BUILD_DEFINITIONS}")

add_library(Tech
${tech}/Pch.cpp
${tech}/tech/Atomic.cpp
${tech}/tech/Core.cpp
${tech}/tech/ImageUtils.cpp
${tech}/tech/Profile.cpp
${tech}/tech/StrUtils.cpp
${tech}/tech/Time.cpp
)
set_target_properties(Tech PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS}")

#####################################
set(AppSources
    ${cmdsrc}/Main.cpp
    ${cmdsrc}/CommandLineOptions.cpp
    ${cmdsrc}/Pch.cpp
    ${cmdsrc}/SettingsText.cpp
)

set(LibrariesToLink ${wxWidgets_LIBRARIES} Tech ImageCompleterLib)

IF(USE_ITK)
  message("using ITK")
  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})

  set(MainAppSources ${AppSources}
    ${cmdsrc}/AppITKImage.cpp
    ${cmdsrc}/AppITKMask.cpp
    )
  set(LibrariesToLink ${LibrariesToLink} ${ITK_LIBRARIES})
else(USE_ITK)
  message("using wx")
  set(MainAppSources ${AppSources}
    ${cmdsrc}/AppWxImage.cpp
    ${cmdsrc}/AppWxMask.cpp
    )
ENDIF(USE_ITK)

add_executable(ImageCompleter ${MainAppSources})
target_link_libraries(ImageCompleter ${LibrariesToLink})
message("Main build definitions: ${MAIN_BUILD_DEFINITIONS}")
message("Core definitions: ${CORE_DEFINITIONS}")
set_target_properties(ImageCompleter PROPERTIES COMPILE_DEFINITIONS "${MAIN_BUILD_DEFINITIONS};${CORE_DEFINITIONS}")

#####################################
# # Special builds
#####################################

# 1 channel float images
# find_package(ITK REQUIRED)
# include(${ITK_USE_FILE})
#
# set(definitions1f "USE_ITK;USE_FLOAT_PIXELS;PIXEL_DIMENSION=1")
# message("ImageCompleter1f definitions: ${CORE_DEFINITIONS};${definitions1f}")
#
# add_library(ImageCompleterLib1f ${ImageCompleterSources})
# set_target_properties(ImageCompleterLib1f PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions1f}")
#
# add_executable(ImageCompleter1f ${AppSources} ${cmdsrc}/AppITKImage.cpp ${cmdsrc}/AppITKMask.cpp)
# target_link_libraries(ImageCompleter1f ${LibrariesToLink} ${ImageCompleterLib1f} ${ITK_LIBRARIES})
# set_target_properties(ImageCompleter1f PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions1f}")


############################
# 3 channel unsigned char images
#Careful, if weights are used the pixel type must be float! (because multiplying a weight of say 2 to a pixel value of 150 will produce 300 which is > 255)
#
# set(definitions3uc "EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET;UNIX;USE_ITK;PIXEL_DIMENSION=3")
#
# add_library(ImageCompleterLib3uc ${ImageCompleterSources})
# set_target_properties(ImageCompleterLib3uc PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions3uc}")
#
# add_executable(ImageCompleter3uc ${AppSources} ${cmdsrc}/AppITKImage.cpp ${cmdsrc}/AppITKMask.cpp)
# target_link_libraries(ImageCompleter3uc ${LibrariesToLink} ${ImageCompleterLib3uc})
# set_target_properties(ImageCompleter3uc PROPERTIES COMPILE_DEFINITIONS "${definitions3uc}")

##############################
# wx build

# set(definitions3uc "PIXEL_DIMENSION=3")
# message("ImageCompleter3uc definitions: ${CORE_DEFINITIONS};${definitions3uc}")
# add_library(ImageCompleterLib3uc ${ImageCompleterSources})
# set_target_properties(ImageCompleterLib3uc PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions3uc}")
#
# add_executable(ImageCompleter3uc ${AppSources} ${cmdsrc}/AppWxImage.cpp ${cmdsrc}/AppWxMask.cpp)
# target_link_libraries(ImageCompleter3uc ${LibrariesToLink} ${ImageCompleterLib3uc})
# set_target_properties(ImageCompleter3uc PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions3uc}")
