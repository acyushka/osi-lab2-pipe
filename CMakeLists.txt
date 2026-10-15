cmake_minimum_required(VERSION 3.28)
project(lab2 C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "This project is for Windows only")
endif()

add_compile_options(-Wall -Wextra)

add_executable(child src/child.c)

add_executable(parent src/parent.c src/common.c)

if(MINGW)
    target_link_libraries(parent PRIVATE user32)
endif()

set_target_properties(child  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(parent PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
