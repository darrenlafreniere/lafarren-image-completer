cmake_minimum_required(VERSION 2.6)

project(ImageCompleter)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include( ${wxWidgets_USE_FILE} )

set(cmdsrc src/image-completer-cmd)
set(ImageCompleterDir src/image-completer-lib/impl)
set(tech src/tech)

include_directories(lib/tacus/ src/image-completer-cmd src/image-completer-lib src/image-completer-lib/api src/image-completer-lib/impl src/image-completer-lib/impl/compositors src/image-completer-lib/impl/energy-calculators src/tech/tech src/tech)

#If you want to enable the patches to be composited using Poisson blending, set this flag to ON (requires TAUCS to be installed).
set(USE_POISSON OFF CACHE BOOL "Use Poisson blending?" FORCE)

#If you want to enable the per-pixel energy calculator to use multiple hardware threads, set this flag to ON.
set(USE_THREADS OFF CACHE BOOL "Use threads?" FORCE)
if(USE_THREADS)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_THREADS")
endif(USE_THREADS)

IF(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX")
ENDIF(UNIX)

###############
add_library(ImageCompleterLib
${ImageCompleterDir}/Compositor.cpp
${ImageCompleterDir}/ConstNodeLabels.cpp
${ImageCompleterDir}/EnergyCalculatorContainer.cpp
${ImageCompleterDir}/HostImageLocal.cpp
${ImageCompleterDir}/Image.cpp
${ImageCompleterDir}/Label.cpp
${ImageCompleterDir}/Mask.cpp
${ImageCompleterDir}/MaskWritable.cpp
${ImageCompleterDir}/NeighborEdge.cpp
${ImageCompleterDir}/Node.cpp
${ImageCompleterDir}/NodeSet.cpp
${ImageCompleterDir}/Pch.cpp
${ImageCompleterDir}/PriorityBp.cpp
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

add_library(Tech
${tech}/Pch.cpp
${tech}/tech/Atomic.cpp
${tech}/tech/Core.cpp
${tech}/tech/ImageUtils.cpp
${tech}/tech/Profile.cpp
${tech}/tech/StrUtils.cpp
${tech}/tech/Time.cpp
)

#if TAUCS is setup correctly, we would want to enable Poisson blending
IF(USE_POISSON)
  set(ImageCompleterSources ${ImageCompleterSources}
  ${prioritybp}/compositors/PatchTypePoisson.cpp
  ${prioritybp}/compositors/PoissonSolver.cpp
  ${prioritybp}/compositors/TaucsMatrix.cpp)
ENDIF(USE_POISSON)

#####################################


add_executable(ImageCompleter ${cmdsrc}/Main.cpp

${cmdsrc}/CommandLineOptions.cpp
${cmdsrc}/Pch.cpp
${cmdsrc}/SettingsText.cpp)
target_link_libraries(ImageCompleter ${wxWidgets_LIBRARIES} Tech ImageCompleterLib)
