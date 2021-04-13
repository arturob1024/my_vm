cmake_minimum_required(VERSION 3.0)

project(
    arturo_vm
    LANGUAGES C CXX
    )

# Formatting C++ sources
add_custom_target(format
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/format.sh
    USES_TERMINAL
    )

add_subdirectory(compiler)
