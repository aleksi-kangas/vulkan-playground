cmake_minimum_required(VERSION 3.24)
project(vulkan_playground)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(engine)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE engine)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# Assets
add_custom_target(assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/assets)
add_dependencies(${PROJECT_NAME} assets)

# Shaders
file(GLOB_RECURSE SHADERS
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag)
foreach (SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV "${CMAKE_CURRENT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders/"
            COMMAND glslc ${SHADER} -o ${SPIRV}
            DEPENDS ${SHADER})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach ()
add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(${PROJECT_NAME} shaders)

