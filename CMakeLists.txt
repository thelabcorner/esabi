cmake_minimum_required(VERSION 3.15)

project(esabi VERSION 0.3.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(esabi INTERFACE)
add_library(esabi::esabi ALIAS esabi)

target_compile_features(esabi INTERFACE c_std_99)
target_include_directories(esabi
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS esabi
    EXPORT esabiTargets
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    cmake/esabiConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/esabiConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/esabi
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/esabiConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/esabiConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/esabiConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/esabi
)

install(
    EXPORT esabiTargets
    FILE esabiTargets.cmake
    NAMESPACE esabi::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/esabi
)
