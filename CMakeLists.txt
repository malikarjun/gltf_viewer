cmake_minimum_required(VERSION 3.8.2)
project(gltf_viewer)

SET(CMAKE_CXX_STANDARD 11)

set(DEFAULT_PROJECT_OPTIONS
        CXX_STANDARD              17
        LINKER_LANGUAGE           "CXX"
        POSITION_INDEPENDENT_CODE ON
        )


include_directories(include
        libraries/glad
        libraries/glfw/include
        libraries/glm
        libraries/tiny_gltf/include
        ) # -I flags for compiler

add_subdirectory(libraries/glfw)
add_subdirectory(libraries/glad)

add_executable(gltf_viewer
        libraries/tiny_gltf/src/tiny_gltf.cc
        camera.cpp
        shaders.cpp
        window.cpp
        main.cpp
        )

target_link_libraries(gltf_viewer glfw glad)   # -l flags for linking prog target
#set_target_properties( main PROPERTIES COMPILE_FLAGS "-w" )