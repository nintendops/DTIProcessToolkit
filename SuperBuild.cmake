#-----------------------------------------------------------------------------
# Superbuild option(s)
#-----------------------------------------------------------------------------
option(BUILD_STYLE_UTILS "Build uncrustify, cppcheck, & KWStyle" OFF)
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_Uncrustify "Use system Uncrustify program" OFF
  "BUILD_STYLE_UTILS" OFF
  )
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_KWStyle "Use system KWStyle program" OFF
  "BUILD_STYLE_UTILS" OFF
  )
CMAKE_DEPENDENT_OPTION(
  USE_SYSTEM_Cppcheck "Use system Cppcheck program" OFF
  "BUILD_STYLE_UTILS" OFF
  )

option(EXECUTABLES_ONLY "Build the tools and the tools' libraries statically" ON)
#-----------------------------------------------------------------------------
# Extension configuration
#-----------------------------------------------------------------------------

########Depending if it is an extension or a Superbuild
if( DTIProcess_BUILD_SLICER_EXTENSION )
  find_package(Slicer REQUIRED)
  set( Slicer_USE_PYTHONQT FALSE )
  set( USE_SYSTEM_ITK ON CACHE BOOL "Build using an externally defined version of ITK" FORCE )
  set( USE_SYSTEM_VTK ON CACHE BOOL "Build using an externally defined version of VTK" FORCE )
  #VTK_VERSION_MAJOR is define but not a CACHE variable
  set( VTK_VERSION_MAJOR ${VTK_VERSION_MAJOR} CACHE STRING "Choose the expected VTK major version to build Slicer (5 or 6).")
  set( USE_SYSTEM_SlicerExecutionModel ON CACHE BOOL "Build using an externally defined version of SlicerExecutionModel" FORCE )
endif()

option(USE_SYSTEM_ITK "Build using an externally defined version of ITK" OFF)
option(USE_SYSTEM_SlicerExecutionModel "Build using an externally defined version of SlicerExecutionModel"  OFF)
option(USE_SYSTEM_VTK "Build using an externally defined version of VTK" OFF)

#------------------------------------------------------------------------------
# ${PRIMARY_PROJECT_NAME} dependency list
#------------------------------------------------------------------------------
#Compilation options for ITK if USE_SYSTEM_ITK is set to OFF (Superbuild)
set(ITK_EXTERNAL_NAME ITKv4)
set(${PRIMARY_PROJECT_NAME}_BUILD_DICOM_SUPPORT ON)
set( USE_ITK_Module_MGHIO ON )
set( ${PRIMARY_PROJECT_NAME}_BUILD_TIFF_SUPPORT ON )
set( ${PRIMARY_PROJECT_NAME}_BUILD_JPEG_SUPPORT ON )
set( ${PRIMARY_PROJECT_NAME}_BUILD_ZLIB_SUPPORT ON )
set( ${PRIMARY_PROJECT_NAME}_BUILD_FFTW_SUPPORT ON )

set(${PRIMARY_PROJECT_NAME}_DEPENDENCIES ${ITK_EXTERNAL_NAME} SlicerExecutionModel VTK )

if(BUILD_STYLE_UTILS)
  list(APPEND ${PRIMARY_PROJECT_NAME}_DEPENDENCIES Cppcheck KWStyle Uncrustify)
endif()




if(${PRIMARY_PROJECT_NAME}_USE_QT)
  list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
    ${PRIMARY_PROJECT_NAME}_USE_QT:BOOL
    QT_QMAKE_EXECUTABLE:PATH
    QT_MOC_EXECUTABLE:PATH
    QT_UIC_EXECUTABLE:PATH
    )
endif()

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
set(extProjName ${PRIMARY_PROJECT_NAME})
set(proj        ${PRIMARY_PROJECT_NAME})

List( LENGTH ${PRIMARY_PROJECT_NAME}_DEPENDENCIES dependencies_size )
if( dependencies_size GREATER 0 )
  SlicerMacroCheckExternalProjectDependency(${PRIMARY_PROJECT_NAME})
endif()
#-----------------------------------------------------------------------------
# Set CMake OSX variable to pass down the external project
#-----------------------------------------------------------------------------
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()


#-----------------------------------------------------------------------------
# Add external project CMake args
#-----------------------------------------------------------------------------
list(APPEND ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARS
  BUILD_TESTING:BOOL
  ITK_VERSION_MAJOR:STRING
  ITK_DIR:PATH
  VTK_DIR:PATH
  GenerateCLP_DIR:PATH
  SlicerExecutionModel_DIR:PATH
  )

_expand_external_project_vars()
set(COMMON_EXTERNAL_PROJECT_ARGS ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})


if(verbose)
  message("Inner external project args:")
  foreach(arg ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_ARGS})
    message("  ${arg}")
  endforeach()
endif()

string(REPLACE ";" "^" ${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES "${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES}")

if(verbose)
  message("Inner external project argnames:")
  foreach(argname ${${CMAKE_PROJECT_NAME}_SUPERBUILD_EP_VARNAMES})
    message("  ${argname}")
  endforeach()
endif()

#------------------------------------------------------------------------------
# Configure and build
#------------------------------------------------------------------------------
  set(proj ${PRIMARY_PROJECT_NAME})
  ExternalProject_Add(${proj}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
    BINARY_DIR ${proj}-build
    DEPENDS ${${PRIMARY_PROJECT_NAME}_DEPENDENCIES}
    CMAKE_GENERATOR ${gen}
    CMAKE_ARGS
      -DMIDAS_PACKAGE_EMAIL:STRING=${MIDAS_PACKAGE_EMAIL}
      -DMIDAS_PACKAGE_API_KEY:STRING=${MIDAS_PACKAGE_API_KEY}
      -DEXTENSION_NAME:STRING=${EXTENSION_NAME}
      -DDTIProcess_SUPERBUILD:BOOL=OFF
      -DINSTALL_RUNTIME_DESTINATION:PATH=${SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION}
      -DINSTALL_LIBRARY_DESTINATION:PATH=${SlicerExecutionModel_DEFAULT_CLI_INSTALL_LIBRARY_DESTINATION}
      ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
      ${COMMON_EXTERNAL_PROJECT_ARGS}
      -DEXECUTABLES_ONLY:BOOL=${EXECUTABLES_ONLY}
      -DEXTENSION_SUPERBUILD_BINARY_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
      -DUSE_SYSTEM_ITK:BOOL=TRUE
      -DUSE_SYSTEM_VTK:BOOL=TRUE
      -DUSE_SYSTEM_SlicerExecutionModel:BOOL=TRUE
    INSTALL_COMMAND ""
  )

## Force rebuilding of the main subproject every time building from super structure
ExternalProject_Add_Step(${proj} forcebuild
    COMMAND ${CMAKE_COMMAND} -E remove
    ${CMAKE_CURRENT_BUILD_DIR}/${proj}-prefix/src/${proj}-stamp/${proj}-build
    DEPENDEES configure
    DEPENDERS build
    ALWAYS 1
  )

#-----------------------------------------------------------------------------
