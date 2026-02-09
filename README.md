# HPC Voxel Engine

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.6-red.svg)
![Build Status](https://github.com/KabilanKumar36/cpp-hpc-voxel-engine/actions/workflows/build_and_test.yml/badge.svg)

A high-performance CPU/GPU voxel engine built from scratch in C++20.
Designed as a foundational framework for Computational Engineering (CAE) simulations, focusing on cache coherency, hardware-accelerated rendering, custom memory allocators and low-level memory management.

![Day 05 Demo](assets/screenshots/demo_day_05.png)

## 🚀 Overview

This project is a technical playground for implementing high-performance graphics concepts without relying on commercial game engines. This engine is architected to support Finite Element Analysis (FEA) and Physics Simulations in later stages.

Core Engineering Pillars:

* **Data-Oriented Design:** Struct-of-Arrays (SoA) layout for voxel data to maximize CPU cache hits.
* **Custom Math Library:** SIMD-ready vectors and matrices.
* **Systems Hardening:** Zero-warning codebase enforced by CI/CD (`/WX` (MSVC) and `-Werror` GCC/Clang).
* **Modern OpenGL (4.6):** Direct State Access (DSA) and optimized buffer streaming for voxel generation.
* **Memory Management:** Leveraging C++20 features and RAII/Move Semantics for deterministic resource lifecycles.
* **Zero-dependency architecture:** Managing memory and resources manually.
* **Memory Ownership:** Strict RAII and Move Semantics (C++20) for deterministic resource management.

## 🛠️ Tech Stack

| Component | Technology | Reasoning |
| :--- | :--- | :--- |
| Language | C++20 | Concepts, Modules, constexpr math. |
| Graphics | OpenGL 4.6 | Core Profile for compute-shader readiness. |
| Build System | CMake 3.23+ | FetchContent for dependency-free setup. |
| Windowing | GLFW 3.4 | Robust cross-platform context management. |
| Loader | Glad | Dynamic loader for OpenGL. |
| Math | Custom SIMD | Hand-rolled Vector/Matrix library for solver integration. |
| Procedural | FastNoiseLite | OpenSimplex2 noise for terrain generation. |
| Asset | stb_image | For Textures generation. |
| Testing | GoogleTest | Physics and math verification. |

📐 System Architecture

The engine follows a strict separation of concerns between the Simulation Loop (Fixed Timestep) and Rendering Loop (Variable Timestep).

graph TD
    App[Application Loop] -->|Poll Events| Input[Input System]
    App -->|Fixed Update| Physics[Physics Engine]
    App -->|Variable Update| Render[Renderer]
    
    subgraph "World Data"
        ChunkMgr[Chunk Manager] -->|Manage| Chunk[Chunk (16^3)]
        Chunk -->|Generate| Mesh[Mesh Builder]
        Mesh -->|Culling| Opt[Face Culling]
    end
    
    Physics -->|Query| ChunkMgr
    Render -->|Draw| ChunkMgr

## ✨ Key Features (Current & Planned)

### ✅ Completed
- [x] **Core Math Library:** 
    - Custom `Vec3` and `Mat4` implementation with `constexpr` and SIMD-ready optimization.
- [x] **High-Performance Rendering:**
    - Hidden Face Removal: Internal and Inter-Chunk occlusion culling (removes ~85% of geometry).
    - Texture Atlasing: Dynamic UV mapping with bitwise face-id logic.
- [x] **Procedural Generation:**
    - Infinite terrain using `FastNoiseLite` (OpenSimplex2) noise.
    - Height-based Biome System (Stone, Dirt, Grass).
- [x] **Engine Core:**
    - Custom Math Library: Vec3 / Mat4 with compiler-intrinsic optimizations.
    - Interactive Camera: Euler-angle based FPS camera.
    - Ray Casting: DDA (Digital Differential Analyzer) for O(1) block picking.
- [x] **Build & CI:**
    - Automated GoogleTest suite for Math/Physics verification.
    - Cross-platform CMake build (Windows/Linux).
- [x] **Render Context:** Robust GLFW window handling and input polling.
- [x] **Interactive Camera:** WASD movement, Mouse Look, and Zoom.
- [x] **Physics Engine:**
    - AABB (Axis-Aligned Bounding Box) Collision Detection.
    - Voxel-aware terrain collision (direct chunk lookup).
    - Gravity and Velocity resolution.
- [x] **Debug Visualization:** Wireframe highlighting of targeted blocks.
- [x] **Interaction:** Block breaking and placing mechanics.
- [x] **Kinematic Character Controller:** Swept-AABB collision resolution against voxel terrain.

### 🚧 In Progress (Physics Transition)
- [ ] **Frustum Culling:** CPU-side culling to optimize GPU workload for large view distances.
- [ ] **ImGui Integration:** Debug dashboard for real-time performance profiling.
- [ ] **Fixed Timestep Loop:** Decoupling simulation logic (60Hz) from rendering (Uncapped).

### 📅 Planned
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

**Coverage:**
* **Physics Logic:** AABB intersection and construction assertions.
* **OpenGL Environment:** Automated invisible window creation for context-dependent tests.
* **Chunk Logic:** Verification of mesh generation and buffer sizing.

## 📂 Project Structure

```text
assets/             # Graphics resources (Shaders, Texture Atlas)
docs/               # Documentation
external/           # Heavy dependencies managed via CMake FetchContent (GLFW, GLAD)
src/
├── app/            # Input Manager and Input Handler
├── core/           # Math (Vec3, Matrix), Camera, Threading, Memory Management, and Base Types
├── physics/        # Physics Engine (AABB, PhysicsSystem, RigidBody)
├── renderer/       # OpenGL 4.6 Wrappers (Shader, Buffer, Texture, VAO and World & Primitive Renderers)
├── world/          # Voxel Logic (Chunk, Mesh Generation, Biome System and Player)
├── vendor/         # Third-party single-header libraries (stb_image, etc.)
└── main.cpp        # Entry point and Application Loop
tests/              # GoogleTest suite (Physics, Math, and Render verification)
vendor/             # Lighter Third-party dependencies (stb_image, etc.)
```

## 📜 License
Distributed under the MIT License. See LICENSE for more information.
