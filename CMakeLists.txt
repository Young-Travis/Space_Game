cmake_minimum_required(VERSION 3.20)
project(SpaceGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(SpaceGame
    src/main.cpp
    src/Game.cpp
    src/GameOverScreen.cpp
    src/TitleScreen.cpp
    src/graphics/Mesh.cpp
    src/graphics/Renderer3D.cpp
    src/graphics/GpuRenderer.cpp
    src/graphics/ParticleExplosion.cpp
    src/graphics/Starfield.cpp
    src/ships/Enemy.cpp
        src/ships/Laser.cpp
    src/ships/Player.cpp
    src/ships/StudentEnemy.cpp
    src/ships/StudentLaser.cpp
    src/ships/StudentPlayer.cpp
    src/ships/Ship.cpp
    src/cockpit/cockpit.cpp
)

target_include_directories(SpaceGame PRIVATE include)
target_compile_definitions(
    SpaceGame
    PRIVATE
        SPACEGAME_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/font.ttf"
)

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
