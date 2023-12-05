cmake_minimum_required(VERSION 3.22)

project(checked_shared_ptr)

set(WARNING_FLAGS "-Wno-unused-variable -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -std=c++17")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CSP_TEST OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

include_directories(include)

if(${ENABLE_CSP_TEST})
    add_subdirectory(test)
endif()

file(GLOB SRC_FILES "include/*.hpp")

add_library(${PROJECT_NAME} INTERFACE ${SRC_FILES})