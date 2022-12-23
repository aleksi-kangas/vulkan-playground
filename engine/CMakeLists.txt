cmake_minimum_required(VERSION 3.24)
project(engine)

set(CMAKE_CXX_STANDARD 20)
include(FetchContent)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
        GIT_TAG 2f947710aeedea2089ade2b62ef669521d90f2ef
)
FetchContent_MakeAvailable(tinyobjloader)

add_library(${PROJECT_NAME}
        include/engine/application.h src/application.cpp
        include/engine/buffer.h src/buffer.cpp
        include/engine/camera.h src/camera.cpp
        include/engine/device.h src/device.cpp
        include/engine/graphics_pipeline.h src/graphics_pipeline.cpp
        include/engine/math.h
        include/engine/mesh.h src/mesh.cpp
        include/engine/model.h src/model.cpp
        include/engine/renderer.h src/renderer.cpp
        include/engine/swap_chain.h src/swap_chain.cpp
        include/engine/texture.h src/texture.cpp
        include/engine/transform.h src/transform.cpp
        include/engine/uniforms.h
        include/engine/utils.h src/utils.cpp
        include/engine/vertex.h src/vertex.cpp
        include/engine/window.h src/window.cpp

        include/engine/systems/model_render_system.h src/systems/model_render_system.cpp
        include/engine/systems/point_light_render_system.h src/systems/point_light_render_system.cpp
        )
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE lib)
target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        tinyobjloader
        )
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
