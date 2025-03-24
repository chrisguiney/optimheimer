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
find_package(Vulkan 1.3 REQUIRED)

set(SQLITE3_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ext/sqlite")
set(SQLITE3_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/ext/sqlite")
set(SQLITE3_SOURCE "${SQLITE3_BUILD_DIR}/sqlite3.c"
        vk.c
        vk.h)
set(SQLITE3_EXE "${SQLITE3_BUILD_DIR}/sqlite3")

add_definitions(-DSQLITE3_ENABLE_ICU)

add_custom_command(
        OUTPUT "${SQLITE3_BUILD_DIR}/sqlite3.c"
        WORKING_DIRECTORY ${SQLITE3_BUILD_DIR}
        COMMAND "mkdir" "-p" ${SQLITE3_BUILD_DIR}
        COMMAND "${SQLITE3_SOURCE_DIR}/configure"
        "--prefix=${SQLITE3_BUILD_DIR}"
        "--enable-all"
        "--with-icu-config=auto"
        COMMAND "make" "sqlite3.c"
)

add_custom_command(
    OUTPUT "oph.db"
    COMMAND ${SQLITE3_EXE}
)

# add_custom_target("sqlite3"
#        WORKING_DIRECTORY ${SQLITE3_BUILD_DIR}
#        DEPENDS ${SQLITE3_SOURCE}
#)

add_executable(optimheimer
        main.c
        device.c
        nuklear.c
        app.c
        sys.c
        allocator.c
        ${SQLITE3_SOURCE}
)


target_include_directories(
        optimheimer
        PRIVATE
        ${SQLITE3_BUILD_DIR}
)

target_link_libraries(
        optimheimer
        PRIVATE
        m
        OpenGL::GL
        glfw
        Vulkan::Vulkan
        sqlite3
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
                COMMAND "${CMAKE_CURRENT_BINARY_DIR}/ext/shaderc/glslc/glslc" ${_shader} "-o" ${_shader_out_name}
                VERBATIM
        )
    endforeach ()
    add_custom_target("build_${SHADER_TYPE}_shaders" DEPENDS ${_output_shaders} glslc_exe)
endfunction()


set(GFX_VERTEX_SHADER_SOURCES
        "assets/shader.vert"
)

set(GFX_FRAGMENT_SHADER_SOURCES
        "assets/shader.frag"
)

add_shaders(gfx_vertex ${GFX_VERTEX_SHADER_SOURCES})
add_shaders(gfx_fragment ${GFX_FRAGMENT_SHADER_SOURCES})