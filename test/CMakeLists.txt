cmake_minimum_required(VERSION 3.2)

project(checked_shared_ptr_test)

include_directories(include)

find_package(GTest REQUIRED)

file(GLOB SRC_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${GTEST_LIBRARIES} 
                    pthread
                    gtest_main)
