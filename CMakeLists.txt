cmake_minimum_required(VERSION 3.30)
project(optimheimer C)

set(CMAKE_C_STANDARD 23)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

add_subdirectory(ext/glfw)

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