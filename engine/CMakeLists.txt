cmake_minimum_required(VERSION 3.24)
project(engine)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

FetchContent_Declare(
        vk-bootstrap
        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
        GIT_TAG v0.6
)
FetchContent_MakeAvailable(vk-bootstrap)

add_library(${PROJECT_NAME}
        include/engine/window.h src/window.cpp
        )
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        vk-bootstrap
        )
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
