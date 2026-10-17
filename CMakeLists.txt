cmake_minimum_required(VERSION 3.28)

# Toolchain and compile_commands.json are set only when built as the top level.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  option(CMAKE_EXPORT_COMPILE_COMMANDS "Export compile_commands.json." ON)
  if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "Use the vcpkg package manager")
  endif()
endif()

project(maboroutu_appkit
    VERSION 0.0.0.1
    DESCRIPTION "Application-side helpers for maboroutu."
    HOMEPAGE_URL "https://github.com/Eigen-0x41/maboroutu-appkit"
    LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

option(${PROJECT_NAME}_SHARED_LIBS "Build ${PROJECT_NAME} as a shared library." OFF)

if(${PROJECT_NAME}_SHARED_LIBS)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()
# The modules depend only on the standard library. The alias keeps the
# maboroutu:: namespace so that the target name is stable for users.
add_library(maboroutu::appkit ALIAS ${PROJECT_NAME})
# Keep the installed name identical to the alias (maboroutu::appkit).
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME appkit)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

file(GLOB_RECURSE ${PROJECT_NAME}_MODULE_INTERFACE CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.cppm")
file(GLOB_RECURSE ${PROJECT_NAME}_MODULE_IMPLEMENTATION CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/src/*.cpp")
target_sources(${PROJECT_NAME}
  PUBLIC
  FILE_SET ${PROJECT_NAME}_FILE_SET
  TYPE CXX_MODULES
  BASE_DIRS ${PROJECT_SOURCE_DIR}/src
  FILES ${${PROJECT_NAME}_MODULE_INTERFACE})
target_sources(${PROJECT_NAME}
  PRIVATE
  ${${PROJECT_NAME}_MODULE_IMPLEMENTATION})

### Install
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
  EXPORT
    ${PROJECT_NAME}-targets
  LIBRARY DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION
    ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION
    ${CMAKE_INSTALL_BINDIR}
  FILE_SET ${PROJECT_NAME}_FILE_SET DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  CXX_MODULES_BMI DESTINATION "")

install(EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE maboroutu::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})

### package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion)
file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
  "@PACKAGE_INIT@\n"
  "include(\"\${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@-targets.cmake\")\n"
  "check_required_components(\"@PROJECT_NAME@\")")
configure_package_config_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
