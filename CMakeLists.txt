cmake_minimum_required(VERSION 2.6)

project(ImageCompleter)

########### Eigen ##############
include_directories(/media/portable/src/Eigen /media/portable/src/Eigen/unsupported)

set(CMAKE_MODULE_PATH /media/portable/src/Eigen/cmake)

FIND_PACKAGE(Umfpack REQUIRED)
FIND_PACKAGE(BLAS REQUIRED)

include_directories(${UMFPACK_INCLUDES})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")
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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_POISSON")
endif(USE_POISSON)

#If you want to enable the per-pixel energy calculator to use multiple hardware threads, set this flag to ON.
set(USE_THREADS OFF CACHE BOOL "Use threads?" FORCE)
if(USE_THREADS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_THREADS")
endif(USE_THREADS)

IF(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX")
ENDIF(UNIX)

set(USE_ITK OFF CACHE BOOL "Use ITK?")

set(USE_FLOAT_PIXELS OFF CACHE BOOL "Use float pixels?")
IF(USE_FLOAT_PIXELS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_FLOAT_PIXELS")
ENDIF(USE_FLOAT_PIXELS)

set(PIXEL_DIMENSION 3 CACHE STRING "Dimensionality of pixels?") #cmake doesn't have an "INT" variable type, so treat it as a string
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPIXEL_DIMENSION=${PIXEL_DIMENSION}")

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
${ImageCompleterDir}/MaskScalable.cpp
${ImageCompleterDir}/MaskWritable.cpp
${ImageCompleterDir}/NeighborEdge.cpp
${ImageCompleterDir}/Node.cpp
${ImageCompleterDir}/NodeSet.cpp
${ImageCompleterDir}/Pch.cpp
${ImageCompleterDir}/PriorityBpRunner.cpp
${ImageCompleterDir}/ScalableDebugging.cpp
${ImageCompleterDir}/Settings.cpp

${ImageCompleterDir}/compositors/CompositorRoot.cpp
${ImageCompleterDir}/compositors/CompositorUtils.cpp
${ImageCompleterDir}/compositors/ImageFloat.cpp
${ImageCompleterDir}/compositors/OutputBlenderDebugSoftMaskIntensity.cpp
${ImageCompleterDir}/compositors/OutputBlenderSoftMask.cpp
${ImageCompleterDir}/compositors/PatchBlenderNone.cpp
${ImageCompleterDir}/compositors/PatchBlenderPriority.cpp
${ImageCompleterDir}/compositors/PatchTypeDebugPatchOrder.cpp
${ImageCompleterDir}/compositors/PatchTypeNormal.cpp

${ImageCompleterDir}/energy-calculators/EnergyCalculatorFft.cpp
${ImageCompleterDir}/energy-calculators/EnergyCalculatorFftUtils.cpp
${ImageCompleterDir}/energy-calculators/EnergyCalculatorPerPixel.cpp

)

add_library(ImageCompleterLib ${ImageCompleterSources})

add_library(Tech
${tech}/Pch.cpp
${tech}/tech/Atomic.cpp
${tech}/tech/Core.cpp
${tech}/tech/ImageUtils.cpp
${tech}/tech/Profile.cpp
${tech}/tech/StrUtils.cpp
${tech}/tech/Time.cpp
)

#####################################

# We can only use the wxImage class if pixels are unsigned char and length 3
if(USE_FLOAT_PIXELS EQUAL OFF)
MESSAGE("Building Wx example")
  add_executable(ImageCompleter
    ${cmdsrc}/Main.cpp
    ${cmdsrc}/AppWxImage.cpp
    ${cmdsrc}/AppWxMask.cpp
    ${cmdsrc}/CommandLineOptions.cpp
    ${cmdsrc}/Pch.cpp
    ${cmdsrc}/SettingsText.cpp)

  set(LibrariesToLink ${wxWidgets_LIBRARIES} Tech ImageCompleterLib)

  IF(USE_POISSON)
    set(LibrariesToLink ${LibrariesToLink} umfpack)
  ENDIF(USE_POISSON)

  target_link_libraries(ImageCompleter ${LibrariesToLink})
endif(USE_FLOAT_PIXELS EQUAL OFF)

######################################
# optional ITK demo
IF(USE_ITK)
  MESSAGE("Building ITK example")
  find_package(ITK REQUIRED)
  include(${ITK_USE_FILE})

  add_executable(ImageCompleterITK
    ${cmdsrc}/Main.cpp
    ${cmdsrc}/AppITKImage.cpp
    ${cmdsrc}/AppITKMask.cpp
    ${cmdsrc}/CommandLineOptions.cpp
    ${cmdsrc}/Pch.cpp
    ${cmdsrc}/SettingsText.cpp
    )

  set(LibrariesToLinkITK ${wxWidgets_LIBRARIES} Tech ImageCompleterLib)

  IF(USE_POISSON)
    set(LibrariesToLinkITK ${LibrariesToLinkITK} umfpack)
  ENDIF(USE_POISSON)

  target_link_libraries(ImageCompleterITK ${wxWidgets_LIBRARIES} ${ITK_LIBRARIES} ${LibrariesToLinkITK})
ENDIF(USE_ITK)