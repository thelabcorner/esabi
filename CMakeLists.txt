cmake_minimum_required(VERSION 3.15)

project(esabi VERSION 0.3.1 LANGUAGES NONE)

include(CMakePackageConfigHelpers)

add_library(esabi INTERFACE)
add_library(esabi::esabi ALIAS esabi)

set(ESABI_INSTALL_INCLUDEDIR "include" CACHE STRING "Install directory for esabi headers")
set(ESABI_INSTALL_CMAKEDIR "lib/cmake/esabi" CACHE STRING "Install directory for esabi CMake package files")

target_include_directories(esabi
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${ESABI_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS esabi
    EXPORT esabiTargets
)

install(
    DIRECTORY include/
    DESTINATION ${ESABI_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    cmake/esabiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/esabiConfig.cmake
    INSTALL_DESTINATION ${ESABI_INSTALL_CMAKEDIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/esabiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMinorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/esabiConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/esabiConfigVersion.cmake
    DESTINATION ${ESABI_INSTALL_CMAKEDIR}
)

install(
    EXPORT esabiTargets
    FILE esabiTargets.cmake
    NAMESPACE esabi::
    DESTINATION ${ESABI_INSTALL_CMAKEDIR}
)

install(
    FILES LICENSE
    DESTINATION share/licenses/esabi
)
