cmake_minimum_required(VERSION 3.23)

project(HPC_Voxel_Engine
VERSION 0.1.0
LANGUAGES CXX C
)

#Enforce C++ 20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#Dependencies
# --------------------------------------------------------
# 1. GLFW (Automatic via FetchContent)
# --------------------------------------------------------
include(FetchContent)
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

# --------------------------------------------------------
# 2. GLAD (Manual Vendor Build)
# We treat GLAD as a library so we don't have to re-compile it every time
# --------------------------------------------------------
add_library(glad STATIC "external/glad/src/glad.c")
target_include_directories(glad PUBLIC "external/glad/include")

# --------------------------------------------------------

#locate sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
#Executables
add_executable(VoxelEngine ${SOURCES})
#include headers
target_include_directories(VoxelEngine PRIVATE src)

target_link_libraries(VoxelEngine 
PRIVATE
glfw
glad)

#Output Directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#Add Subdirectories
#add_subdirectory(external)
#add_subdirectory(src)