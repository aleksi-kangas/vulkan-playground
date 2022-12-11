cmake_minimum_required(VERSION 3.24)
project(engine)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_library(${PROJECT_NAME}
        include/engine/application.h src/application.cpp
        include/engine/buffer.h src/buffer.cpp
        include/engine/device.h src/device.cpp
        include/engine/graphics_pipeline.h src/graphics_pipeline.cpp
        include/engine/math.h
        include/engine/mesh.h src/mesh.cpp
        include/engine/model.h src/model.cpp
        include/engine/renderer.h src/renderer.cpp
        include/engine/swap_chain.h src/swap_chain.cpp
        include/engine/utils.h src/utils.cpp
        include/engine/vertex.h src/vertex.cpp
        include/engine/window.h src/window.cpp
        )
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        )
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
