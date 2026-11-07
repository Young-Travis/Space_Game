# Space Frontier — Title Screen

A small SFML 3 / C++17 title-screen prototype featuring a rotating 3D wireframe fighter and a keyboard-controlled PLAY / EXIT menu.

## Requirements

- CMake 3.20+
- A C++17 compiler
- SFML 3 with Graphics, Window, and System components

## Build on Linux (CachyOS / Arch)

Install the build tools and SFML:

```bash
sudo pacman -S --needed cmake gcc sfml
```

Configure and build:

```bash
cmake -S . -B build
cmake --build build -j
```

Run:

```bash
./build/SpaceGame
```

## Controls

- Up / Down: change menu selection
- Enter: activate selected option
- Escape: exit

## Project structure

- `Game`: owns the window and runs the main loop.
- `TitleScreen`: owns title-screen state, input, animation, and menu drawing.
- `Mesh`: stores the fighter's local-space vertices and edges.
- `Renderer3D`: transforms, projects, clips, and draws wireframe edges.

The renderer is deliberately small and CPU-based. SFML only draws the final 2D line segments.
# Space_Game
