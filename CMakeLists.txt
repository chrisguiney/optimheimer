cmake_minimum_required(VERSION 3.30)
project(optimheimer C)

set(CMAKE_C_STANDARD 23)
set(SHADERS_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/")

file(GLOB GFX_VERTEX_SHADERS           "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert")
file(GLOB GFX_TESSELATION_CTRL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tesc")
file(GLOB GFX_TESSELATION_EVAL_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.tese")
file(GLOB GFX_GEOMETRY_SHADERS         "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.geom")
file(GLOB GFX_FRAGMENT_SHADERS         "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag")
file(GLOB GFX_FRAGMENT_SHADERS         "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.comp")

file(GLOB RAY_GENERATION_SHADERS   "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rgen")
file(GLOB RAY_INTERSECTION_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rgen")
file(GLOB RAY_ANY_HIT_SHADERS      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rahit")
file(GLOB RAY_CLOSEST_HIT_SHADERS  "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rchit")
file(GLOB RAY_MISS_SHADERS         "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rmiss")
file(GLOB RAY_CALLABLE_SHADERS     "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.rcall")

add_subdirectory(ext/googletest)
add_subdirectory(ext/SPIRV-Headers)
add_subdirectory(ext/SPIRV-Tools)
add_subdirectory(ext/glslang)
add_subdirectory(ext/shaderc)
add_subdirectory(ext/glfw)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(optimheimer
    main.c
    device.c
    nuklear.c
    app.c
)

target_include_directories(
        optimheimer
        PRIVATE
)

target_link_libraries(
        optimheimer
        PRIVATE
        m
        OpenGL::GL
        glfw
        Vulkan::Vulkan
)

# see also:
# https://gist.github.com/mrtrizer/8047a79294e59cd2a1f8aa06795e756a
function(add_shaders SHADER_TYPE SHADERS)
    unset(_output_shaders)
    foreach (_shader ${SHADERS})
        message("adding ${_shader}")
        get_filename_component(_shader_name ${_shader} NAME)
        set(_shader_out_name "${SHADERS_OUT_DIR}/${_shader_name}.spv")
        list(APPEND _output_shaders ${_shader_out_name})
        add_custom_command(
                MAIN_DEPENDENCY ${_shader}
                OUTPUT ${_shader_out_name}
                COMMAND "${CMAKE_BINARY_DIR}/ext/shaderc/glslc/glslc" ${_shader} "-o" ${_shader_out_name}
                VERBATIM
        )
    endforeach ()
    add_custom_target("build_${SHADER_TYPE}_shaders" DEPENDS ${_output_shaders} glslc_exe)
endfunction()

#message("adding ${GFX_VERTEX_SHADERS}")
add_shaders(vertex ${GFX_VERTEX_SHADERS})