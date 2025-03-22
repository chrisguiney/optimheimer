cmake_minimum_required(VERSION 3.30)
project(optimheimer C)

set(CMAKE_C_STANDARD 23)

add_subdirectory(ext/googletest)
add_subdirectory(ext/SPIRV-Headers)
add_subdirectory(ext/SPIRV-Tools)
add_subdirectory(ext/glslang)
add_subdirectory(ext/shaderc)
add_subdirectory(ext/glfw)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(optimheimer
        main.c
        device.c
        nuklear.c
        app.c
        sys.c
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
function(add_shaders SHADER_TYPE SHADER_SOURCES)
    unset(_output_shaders)
    foreach (_shader ${SHADER_SOURCES})
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

set(GFX_VERTEX_SHADER_SOURCES
        "shaders/shader.vert"
)

set(GFX_FRAGMENT_SHADER_SOURCES
        "shaders/shader.frag"
)

add_shaders(gfx_vertex ${GFX_VERTEX_SHADER_SOURCES})
add_shaders(gfx_fragment ${GFX_FRAGMENT_SHADER_SOURCES})