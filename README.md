# HPC Voxel Engine

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.6-red.svg)
![Build Status](https://github.com/KabilanKumar36/cpp-hpc-voxel-engine/actions/workflows/cmake.yml/badge.svg)

A high-performance CPU/GPU voxel engine built from scratch in C++20.
Designed to explore computational geometry, hardware-accelerated rendering, and low-level memory management.

![Day 03 Demo](assets/screenshots/demo_day_03.png)

## 🚀 Overview

This project is a technical playground for implementing high-performance graphics concepts without relying on heavy game engines. The core focus is on:
* **Data-Oriented Design** for cache efficiency.
* **Custom Math Library** (SIMD-ready vectors and matrices).
* **Modern OpenGL (4.6)** utilizing Compute Shaders for voxel generation.
* **Zero-dependency architecture** (managing memory and resources manually).
* **Strict Compliance:** Warning-free codebase enforced by CI/CD (`/WX` on MSVC, `-Werror` on GCC).

## 🛠️ Tech Stack

* **Language:** C++20 (Concepts, Modules, constexpr math)
* **Graphics API:** OpenGL 4.6 (Core Profile)
* **Build System:** CMake 3.23+ (FetchContent for dependencies)
* **Windowing:** GLFW 3.4
* **Loader:** GLAD
* **Assets/Math:** stb_image (Textures), FastNoiseLite (Procedural Generation)
* **Testing:** GoogleTest

## ✨ Key Features (Current & Planned)

### ✅ Completed
- [x] **Core Math Library:** Custom `Vec3` implementation with `constexpr` optimization.
- [x] **Render Context:** Robust GLFW window handling and input polling.
- [x] **Interactive Camera:** WASD movement, Mouse Look, and Zoom.
- [x] **Chunk System:** 16x16x16 Voxel Mesh Generation.
- [x] **Procedural Terrain:** Infinite terrain generation using `FastNoiseLite` (OpenSimplex2).
- [x] **Biome System:** Height-based block assignment (Grass, Dirt, Stone).
- [x] **Texture Atlas:** Efficient single-texture rendering with dynamic UV mapping (256x256 grid).
- [x] **Smart Texturing:**
    - Dynamic Face Assignment: Correctly renders Grass Top, Dirt Bottom, and Side transitions.
    - "Cave Ceiling" Logic: Dirt blocks appearing as Stone when viewed from below.
- [x] **Optimization:** Greedy Face Culling (occlusion culling) for rendering efficiency.
- [x] **Build System:** Self-contained CMake setup with automated dependency management.
- [x] **Physics Engine:**
    - AABB (Axis-Aligned Bounding Box) Collision Detection.
    - Voxel-aware terrain collision (direct chunk lookup).
    - Gravity and Velocity resolution.
- [x] **CI/CD Pipeline:** Automated Linux builds and Unit Testing via GitHub Actions.

### 🚧 In Progress (Day 05 Roadmap)
- [ ] **Ray Casting:** DDA (Digital Differential Analyzer) Algorithm for precise block selection.
- [ ] **Interaction:** Block breaking and placing mechanics.
- [ ] **Debug Visualization:** Wireframe highlighting of targeted blocks.

### 📅 Planned
- [ ] **Infinite World:** Dynamic chunk paging and multithreaded generation.
- [ ] **Lighting Engine:** Ambient Occlusion (AO) and Day/Night cycle.
- [ ] **Water Simulation:** Transparent rendering pass.

## 📦 Build Instructions

### Prerequisites
* **C++ Compiler:** MSVC (Visual Studio 2022) or GCC 11+
* **CMake:** Version 3.23 or higher
* **GPU Drivers:** Must support OpenGL 4.6

### Steps
1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/KabilanKumar36/cpp-hpc-voxel-engine.git](https://github.com/KabilanKumar36/cpp-hpc-voxel-engine.git)
    cd cpp-hpc-voxel-engine
    ```

2.  **Generate Project Files:**
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

3.  **Build:**
    Open the generated solution in Visual Studio or run:
    ```bash
    cmake --build . --config Release
    ```

4.  **Run:**
    ```bash
    ./bin/Release/VoxelEngine.exe
    ```

## 🧪 Testing

The project uses **GoogleTest** for unit testing. The build system automatically fetches the dependency and compiles the test suite.

### Running Tests
1.  **Configure & Build:**
    ```bash
    cmake -B build
    cmake --build build
    ```
2.  **Run the Test Executable:**
    ```bash
    ./build/bin/unit_tests.exe
    ```

**Current Test Coverage:**
* **Physics Logic:** AABB intersection and construction assertions.
* **OpenGL Environment:** Automated invisible window creation for context-dependent tests.
* **Chunk Logic:** Verification of mesh generation and buffer sizing.

## 📂 Project Structure

```text
src/
├── core/       # Math (Vec3, Matrix, FastNoiseLite), Memory, and Base Types
├── physics/    # Physics Engine (AABB, PhysicsSystem, RigidBody)
├── renderer/   # OpenGL wrappers (Shader, Buffer, Texture, VAO)
├── world/      # Voxel Data (Chunk, Mesh Generation, Biome Logic)
└── main.cpp    # Entry point, Window management, and Game Loop
external/       # Vendored dependencies (GLAD)
assets/         # Shaders and Textures (Texture Atlas)
tests/          # Unit Tests (Physics, Core, Rendering)
📜 License
Distributed under the MIT License. See LICENSE for more information.
