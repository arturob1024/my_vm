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

# Set the global C++ version
# TODO: Go to C++20
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

add_compile_options(
    -Wall -Wextra -Werror -Wconversion -Wmissing-declarations
    )

# TODO: Add a common header for things both the compiler and VM need

# Include the subparts of the project
add_subdirectory(compiler)
add_subdirectory(vm)
