cmake_minimum_required(VERSION 2.6)

project(ImageCompleter)

################ Setup Libraries #################

# Tell CMake to also look in the ImageCompleter source directory to find some .cmake files (Eigen3, Umfpack, and FFTW)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

#Eigen
# If you used a prefix so that 'make install' didn't go to a standard location (like /usr/local, etc)
# then you must set EIGEN3_INCLUDE_DIR to [prefix]/include/eigen3
FIND_PACKAGE(Eigen3) #requires FindEigen3.cmake to be in the source directory
include_directories(${EIGEN3_INCLUDE_DIR} ${EIGEN3_INCLUDE_DIR}/unsupported/)

#if we will be using Eigen, it will be with Umfpack and BLAS
if(EIGEN_FOUND)
  #UMFPACK
  FIND_PACKAGE(Umfpack REQUIRED)
  include_directories(${UMFPACK_INCLUDES})
  include_directories(${EIGEN3_INCLUDE_DIR}/unsupported/)

  #BLAS
  FIND_PACKAGE(BLAS REQUIRED)
endif(EIGEN_FOUND)

#FFTW
find_package(FFTW REQUIRED)

#ITK
find_package(ITK)
if(ITK_FOUND)
  include(${ITK_USE_FILE})
endif(ITK_FOUND)

list(APPEND CORE_DEFINITIONS "EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET")
# must link to umfpack

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Compiled in Release mode")
  SET(DEBUGMODE OFF)
else(CMAKE_BUILD_TYPE STREQUAL "Release")
  SET(DEBUGMODE ON)
  list(APPEND MAIN_BUILD_DEFINITIONS "_DEBUG")
  message("Compiled in Debug mode")
endif(CMAKE_BUILD_TYPE STREQUAL "Release")
################################

# If you don't have wx2.9 installed to a system location, you must set
#wxWidgets_CONFIG_EXECUTABLE=/home/doriad/bin/wx2.9/wx-config
#wxWidgets_wxrc_EXECUTABLE=/home/doriad/bin/wx2.9/utils/wxrc/wxrc

find_package(wxWidgets COMPONENTS core base REQUIRED)
include( ${wxWidgets_USE_FILE} )

################ Setup ImageCompleter #################
# Set include paths for ImageCompleter
set(cmdsrc src/image-completer-cmd)
set(ImageCompleterDir src/image-completer-lib/impl)
set(tech src/tech)
include_directories(src/image-completer-cmd src/image-completer-lib src/image-completer-lib/api 
src/image-completer-lib/impl src/image-completer-lib/impl/compositors src/image-completer-lib/impl/energy-calculators 
src/tech/tech src/tech)

################ Setup User Options #################
#If Eigen was found, the user can choose to composite patches using Poisson blending
if(EIGEN_FOUND)
  set(POISSON_COMPOSITING OFF CACHE BOOL "Use Poisson compositing?")
endif(EIGEN_FOUND)

#If you want to enable the per-pixel energy calculator to use multiple hardware threads, set this flag to ON.
set(USE_THREADS OFF CACHE BOOL "Use threads?" FORCE)

#If this flag is true, the MainBuild will be built with ITK instead of WX
set(USE_ITK OFF CACHE BOOL "Use ITK?")


#Some things only become options if using ITK.
if(USE_ITK)
  #If this flag is true, pixels will be read as floats instead of the default of unsigned char.
  set(USE_FLOAT_PIXELS OFF CACHE BOOL "Use float pixels?")

  #ITK can support ND pixels
  set(PIXEL_DIMENSION 3 CACHE STRING "Dimensionality of pixels?") #cmake doesn't have an "INT" variable type, so treat it as a string
endif(USE_ITK)

################ Configure Main Build Based On User Options #################
if(POISSON_COMPOSITING)
  set(MAIN_BUILD_DEFINITIONS "${MAIN_BUILD_DEFINITIONS} POISSON_COMPOSITING;")
  set(CustomBuildLibraries ${CustomBuildLibraries} umfpack)
endif(POISSON_COMPOSITING)

if(USE_THREADS)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_THREADS")
endif(USE_THREADS)

IF(UNIX)
  list(APPEND CORE_DEFINITIONS "UNIX")
ENDIF(UNIX)

IF(USE_ITK)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_ITK")
ENDIF(USE_ITK)

IF(USE_FLOAT_PIXELS)
  list(APPEND MAIN_BUILD_DEFINITIONS "USE_FLOAT_PIXELS")
ENDIF(USE_FLOAT_PIXELS)

list(APPEND MAIN_BUILD_DEFINITIONS "PIXEL_DIMENSION=${PIXEL_DIMENSION}")

################ Create a static library out of the Tech sources #################
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

################ Create a static library out of the ImageCompleter sources #################
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
${ImageCompleterDir}/energy-calculators/EnergyWsst.cpp

)

IF(POISSON_COMPOSITING)
  set(ImageCompleterSources ${ImageCompleterSources}
  ${ImageCompleterDir}/compositors/PatchTypePoisson.cpp
  ${ImageCompleterDir}/compositors/PoissonSolver.cpp)
ENDIF(POISSON_COMPOSITING)

add_library(ImageCompleterLib ${ImageCompleterSources})
target_link_libraries(ImageCompleterLib Tech fftw3f)
set_target_properties(ImageCompleterLib PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${MAIN_BUILD_DEFINITIONS}")

################ Build the demonstration executable with the user selected options #################
set(AppSources
    ${cmdsrc}/Main.cpp
    ${cmdsrc}/CommandLineOptions.cpp
    ${cmdsrc}/Pch.cpp
    ${cmdsrc}/SettingsText.cpp
)

set(LibrariesAlwaysUsed ${wxWidgets_LIBRARIES} Tech ImageCompleterLib)

IF(USE_ITK)
  message("using ITK")

  set(MainAppSources ${AppSources}
    ${cmdsrc}/AppITKImage.cpp
    ${cmdsrc}/AppITKMask.cpp
    )
  set(CustomBuildLibraries ${CustomBuildLibraries} ${ITK_LIBRARIES})
else(USE_ITK)
  message("using wx")
  set(MainAppSources ${AppSources}
    ${cmdsrc}/AppWxImage.cpp
    ${cmdsrc}/AppWxMask.cpp
    )
ENDIF(USE_ITK)

add_executable(ImageCompleter ${MainAppSources})
target_link_libraries(ImageCompleter ${CustomBuildLibraries} ${LibrariesAlwaysUsed})
message("Main build definitions: ${MAIN_BUILD_DEFINITIONS}")
message("Core definitions: ${CORE_DEFINITIONS}")
set_target_properties(ImageCompleter PROPERTIES COMPILE_DEFINITIONS "${MAIN_BUILD_DEFINITIONS};${CORE_DEFINITIONS}")

######################
# # Special builds # #
######################
# 1D float images and 3D unsigned char images are the most common images to be completed. Build these executables regardless of the user settings.

# 1D/channel float images

if(ITK_FOUND)
  list(APPEND definitions1f "USE_ITK;USE_FLOAT_PIXELS;PIXEL_DIMENSION=1")
  if(DEBUGMODE)
    list(APPEND definitions1f "_DEBUG")
  endif(DEBUGMODE)
  message("ImageCompleter1f CORE_DEFINITIONS: ${CORE_DEFINITIONS}")
  message("ImageCompleter1f definitions1f: ${definitions1f}")
  message("ImageCompleter1f LibrariesAlwaysUsed: ${LibrariesAlwaysUsed}")
  message("ImageCompleter1f ImageCompleterSources: ${ImageCompleterSources}")
  message("ImageCompleter1f AppSources: ${AppSources}")

  add_library(ImageCompleterLib1f ${ImageCompleterSources})
  set_target_properties(ImageCompleterLib1f PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions1f}")

  add_executable(ImageCompleter1f ${AppSources} ${cmdsrc}/AppITKImage.cpp ${cmdsrc}/AppITKMask.cpp)
  target_link_libraries(ImageCompleter1f ${LibrariesAlwaysUsed} ${ImageCompleterLib1f} ${ITK_LIBRARIES})
  set_target_properties(ImageCompleter1f PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions1f}")
endif(ITK_FOUND)

############################
# 3D/channel unsigned char images
#Careful, if weights are used the pixel type must be float! (because multiplying a weight of say 2 to a pixel value of 150 will produce 300 which is > 255)

if(ITK_FOUND)
  #set(definitions3uc "EIGEN_YES_I_KNOW_SPARSE_MODULE_IS_NOT_STABLE_YET;UNIX;USE_ITK;PIXEL_DIMENSION=3")
  list(APPEND definitions3uc "UNIX;USE_ITK;PIXEL_DIMENSION=3")
  if(DEBUGMODE)
    list(APPEND definitions3uc "_DEBUG")
  endif(DEBUGMODE)

  add_library(ImageCompleterLib3uc ${ImageCompleterSources})
  set_target_properties(ImageCompleterLib3uc PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitions3uc}")

  add_executable(ImageCompleter3uc ${AppSources} ${cmdsrc}/AppITKImage.cpp ${cmdsrc}/AppITKMask.cpp)
  target_link_libraries(ImageCompleter3uc ${LibrariesAlwaysUsed} ${ImageCompleterLib3uc} ${ITK_LIBRARIES})
  set_target_properties(ImageCompleter3uc PROPERTIES COMPILE_DEFINITIONS "${definitions3uc}")
endif(ITK_FOUND)

##############################
# wx build

#
# set(definitionsWX "PIXEL_DIMENSION=3")
# message("ImageCompleterWX definitions: ${CORE_DEFINITIONS};${definitionsWX}")
# add_library(ImageCompleterLibWX ${ImageCompleterSources})
# set_target_properties(ImageCompleterLibWX PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitionsWX}")
#
# add_executable(ImageCompleterWX ${AppSources} ${cmdsrc}/AppWxImage.cpp ${cmdsrc}/AppWxMask.cpp)
# target_link_libraries(ImageCompleterWX ${LibrariesToLink} ${ImageCompleterLibWX})
# set_target_properties(ImageCompleterWX PROPERTIES COMPILE_DEFINITIONS "${CORE_DEFINITIONS};${definitionsWX}")
