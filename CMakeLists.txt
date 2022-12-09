cmake_minimum_required(VERSION 3.24)
project(vulkan_playground)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE engine)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
