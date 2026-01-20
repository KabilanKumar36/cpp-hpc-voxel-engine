# HPC Voxel Engine

![Language](https://img.shields.io/badge/language-C%2B%2B20-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.6-red.svg)

A high-performance CPU/GPU voxel engine built from scratch in C++20.
Designed to explore computational geometry, hardware-accelerated rendering, and low-level memory management.

## 🚀 Overview

This project is a technical playground for implementing high-performance graphics concepts without relying on heavy game engines. The core focus is on:
* **Data-Oriented Design** for cache efficiency.
* **Custom Math Library** (SIMD-ready vectors and matrices).
* **Modern OpenGL (4.6)** utilizing Compute Shaders for voxel generation.
* **Zero-dependency architecture** (managing memory and resources manually).

## 🛠️ Tech Stack

* **Language:** C++20 (Concepts, Modules, constexpr math)
* **Graphics API:** OpenGL 4.6 (Core Profile)
* **Build System:** CMake 3.23+
* **Windowing:** GLFW (fetched via CMake)
* **Loader:** GLAD

## ✨ Key Features (Current & Planned)

- [x] **Core Math Library:** Custom `Vec3` implementation with `constexpr` optimization.
- [x] **Render Context:** Robust GLFW window handling and input polling.
- [x] **Build System:** Self-contained CMake setup with automated dependency management.
- [ ] **Ray Casting:** CPU-side ray-voxel intersection.
- [ ] **Chunk Management:** Infinite terrain paging system.
- [ ] **GPU Acceleration:** Compute shaders for procedural terrain generation.

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

## 📂 Project Structure

```text
src/
├── core/       # Math (Vec3, Ray), Memory, and Base Types
├── renderer/   # OpenGL wrappers (Shader, Buffer, Texture)
└── app/        # Window management and Main Loop
external/       # Vendored dependencies (GLAD)

📜 License
Distributed under the MIT License. See LICENSE for more information.
