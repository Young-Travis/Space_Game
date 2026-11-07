cmake_minimum_required(VERSION 3.20)
project(SpaceFrontierTitle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(SpaceGame
    src/main.cpp
    src/Game.cpp
    src/TitleScreen.cpp
    src/graphics/Mesh.cpp
    src/graphics/Renderer3D.cpp
    src/graphics/GpuRenderer.cpp
    src/graphics/Starfield.cpp
    src/ships/Enemy.cpp
    src/ships/Ship.cpp
    src/cockpit/cockpit.cpp
)

target_include_directories(SpaceGame PRIVATE include)

target_link_libraries(SpaceGame
    PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        OpenGL::GL
)

if(MSVC)
    target_compile_options(SpaceGame PRIVATE /W4)
else()
    target_compile_options(SpaceGame PRIVATE -Wall -Wextra -Wpedantic)
endif()
