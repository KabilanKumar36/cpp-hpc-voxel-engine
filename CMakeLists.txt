cmake_minimum_required(VERSION 3.23)

project(HPC_Voxel_Engine
VERSION 0.1.0
LANGUAGES CXX C
)

# --------------------------------------------------------
# Global Settings
# --------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

#Download to a permanent folder in your project root
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/external_cache")
#Stop checking for Git updates every time (Saves data!)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
include(FetchContent)

# --------------------------------------------------------
# 1. Dependencies
# --------------------------------------------------------

# --- GLFW ---
message(STATUS "Downloading and configuring GLFW...")
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4  # Lock to a stable version
)
# These flags prevent GLFW from building its own docs/tests/examples (saves time)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# --- GLAD ---
# We treat GLAD as a static library
add_library(glad STATIC "external/glad/src/glad.c")
target_include_directories(glad PUBLIC "external/glad/include")

# --- GoogleTest ---
message(STATUS "Downloading and configuring GoogleTest...")
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE  # <--- Fixes the warning
)
# Force shared CRT for MSVC compatibility (prevents linker warnings/errors)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --------------------------------------------------------
# 2. VoxelCore Library (Shared Logic)
# --------------------------------------------------------
# Gather all source files EXCEPT main.cpp. 
# This allows us to share the engine code between the Game and the Tests.
file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*main\\.cpp$") 

add_library(VoxelCore STATIC ${ENGINE_SOURCES})
target_include_directories(VoxelCore PUBLIC src)
# Link OpenGL dependencies to the Core so Chunk.cpp can use them
target_link_libraries(VoxelCore PUBLIC glad glfw) 

# --------------------------------------------------------
# 3. Main Executable
# --------------------------------------------------------
# Make sure your main file is listed here
add_executable(VoxelEngine "src/main.cpp") 
target_link_libraries(VoxelEngine PRIVATE VoxelCore)

# --------------------------------------------------------
# 4. Unit Tests
# --------------------------------------------------------
enable_testing()

add_executable(unit_tests tests/test_main.cpp)

# Link the VoxelCore library (which contains Chunk.cpp) and GTest
target_link_libraries(unit_tests 
    PRIVATE
    VoxelCore 
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)

#locate sources
#file(GLOB_RECURSE SOURCES "src/*.cpp")
#Executables
#add_executable(VoxelEngine ${SOURCES})
#include headers
#target_include_directories(VoxelEngine PRIVATE src)

#target_link_libraries(VoxelEngine 
#PRIVATE
#glfw
#glad)

#Add Subdirectories
#add_subdirectory(external)
#add_subdirectory(src)