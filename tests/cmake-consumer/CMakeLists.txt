cmake_minimum_required(VERSION 3.15)

project(esabi_cmake_consumer LANGUAGES C CXX)

find_package(esabi 0.3 CONFIG REQUIRED)

add_executable(esabi_c_consumer main.c)
target_link_libraries(esabi_c_consumer PRIVATE esabi::esabi)
set_target_properties(esabi_c_consumer PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)

add_executable(esabi_cpp_consumer main.cpp)
target_link_libraries(esabi_cpp_consumer PRIVATE esabi::esabi)
set_target_properties(esabi_cpp_consumer PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
