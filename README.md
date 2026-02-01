# HPC Voxel Engine

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.6-red.svg)
![Build Status](https://github.com/KabilanKumar36/cpp-hpc-voxel-engine/actions/workflows/build_and_test.yml/badge.svg)

A high-performance CPU/GPU voxel engine built from scratch in C++20.
Designed to explore computational geometry, hardware-accelerated rendering, and low-level memory management.

![Day 05 Demo](assets/screenshots/demo_day_05.png)

## 🚀 Overview

This project is a technical playground for implementing high-performance graphics concepts without relying on heavy game engines. The core focus is on:
* **Data-Oriented Design** for cache efficiency and high-throughput vertex processing.
* **Custom Math Library** (SIMD-ready vectors and matrices).
* **Systems Hardening:** Zero-warning codebase enforced via `/WX` (MSVC) and `-Werror` (GCC/Clang).
* **Modern OpenGL (4.6):** Utilizing Compute Shaders and optimized buffer streaming for voxel generation.
* **Memory Management:** Leveraging C++20 features and RAII/Move Semantics for deterministic resource lifecycles.
* **Zero-dependency architecture** (managing memory and resources manually).
* **Strict Compliance:** Warning-free codebase enforced by CI/CD (`/WX` on MSVC, `-Werror` on GCC).

## 🛠️ Tech Stack

* **Language:** C++20 (Concepts, Modules, constexpr math)
* **Graphics API:** OpenGL 4.6 (Core Profile)
* **Build System:** CMake 3.23+ (FetchContent for zero-manual-install setup)
* **Windowing:** GLFW 3.4
* **Loader:** GLAD
* **Assets/Math:** stb_image (Textures), FastNoiseLite (Procedural Generation)
* **Testing:** GoogleTest for physics and math verification.

## ✨ Key Features (Current & Planned)

### ✅ Completed
- [x] **Core Math Library:** Custom `Vec3` implementation with `constexpr` and SIMD-ready optimization.
- [x] **Optimization (Greedy Face Culling)**: Efficient geometry generation reducing draw calls and vertex count by up to 90%.
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
- [x] **Ray Casting:** DDA (Digital Differential Analyzer) Algorithm for precise block selection.
- [x] **Interaction:** Block breaking and placing mechanics.
- [x] **Debug Visualization:** Wireframe highlighting of targeted blocks.

### 🚧 In Progress (Day 05 Roadmap)
- [ ] **Kinematic Player Controller:** Transitioning from "Spectator Mode" to a physics-bound entity.
- [ ] **AABB Collision Resolution:** Three-pass (X, Y, Z) solver for stable interaction with voxel terrain.
- [ ] **Gravity & Jump Impulse:** Implementation of constant acceleration and grounded state logic.

### 📅 Planned
- [ ] **Infinite World:** Dynamic chunk paging and multithreaded generation.
- [ ] **Lighting Engine:** Ambient Occlusion (AO) and Day/Night cycle.
- [ ] **Water Simulation:** Transparent rendering pass.
- [ ] **Frustum Culling:** CPU-side culling to optimize GPU workload for large view distances.

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
├── core/           # Math (Vec3, Matrix), Memory Management, and Base Types
├── physics/        # Physics Engine (AABB, PhysicsSystem, KinematicBody)
├── renderer/       # OpenGL 4.6 Wrappers (Shader, Buffer, Texture, VAO)
├── world/          # Voxel Logic (Chunk, Mesh Generation, Biome System)
├── vendor/         # Third-party single-header libraries (stb_image, etc.)
└── main.cpp        # Entry point and Application Loop
assets/             # Graphics resources (Shaders, Texture Atlas)
external/           # Heavy dependencies managed via CMake FetchContent (GLFW, GLAD)
tests/              # GoogleTest suite (Physics, Math, and Render verification)
📜 License
Distributed under the MIT License. See LICENSE for more information.
