cmake_minimum_required(VERSION 3.23)

project(HPC_Voxel_Engine
VERSION 0.1.0
LANGUAGES CXX C
)

# --------------------------------------------------------
# 1. Global Hardening
# --------------------------------------------------------
option(ENABLE_ASAN "Enable AddressSanitizer for memory safety" ON)

if(ENABLE_ASAN)
    if(MSVC)
        add_compile_options($<$<CONFIG:Debug>:/fsanitize=address>)
        add_link_options($<$<CONFIG:Debug>:/INCREMENTAL:NO>)
    else()
        add_compile_options(-fsanitize=address -g)
        add_link_options(-fsanitize=address)
    endif()
endif()
# --------------------------------------------------------
# 2. Global Settings
# --------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
# 3. Dependencies
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
target_include_directories(glad SYSTEM PUBLIC "external/glad/include")

# --- ImGUI ---
message(STATUS "Downloading and configuring ImGUI...")
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG docking
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC 
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui SYSTEM PUBLIC 
   ${imgui_SOURCE_DIR}
   ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PRIVATE glfw)

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
# 4. STRICT WARNINGS & SANITIZERS
# --------------------------------------------------------
function(set_strict_warnings target_name)
    if(MSVC)
        # Windows (Visual Studio)
        target_compile_options(${target_name} PRIVATE
            /W4       # Warning Level 4 (High)
            /WX       # Treat Warnings as Errors
            /permissive- # Enforce strict standards conformance
            /D_CRT_SECURE_NO_WARNINGS # Optional: Silence C-style safety warnings if needed
            /MP # Multi processor compilation
        )
    else()
        # Linux / macOS (GCC / Clang)
        target_compile_options(${target_name} PUBLIC
            -Wall
            -Wextra
            -Wpedantic
            -Werror   # The specific flag that fails build on warning
            -Wshadow
            -Wconversion
        )
    endif()
endfunction()

# --------------------------------------------------------
# 5. VoxelCore Library (Shared Logic)
# --------------------------------------------------------
# Gather all source files EXCEPT main.cpp. 
# This allows us to share the engine code between the Game and the Tests.
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
list(FILTER ENGINE_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

set(VENDOR_SOURCES "vendor/stb_image.cpp")
add_library(VoxelCore STATIC ${ENGINE_SOURCES} ${VENDOR_SOURCES})
target_include_directories(VoxelCore PUBLIC src )
# To exclude vendor library for strict check of Warnings
target_include_directories(VoxelCore SYSTEM PUBLIC "${CMAKE_SOURCE_DIR}/vendor")
# Link OpenGL dependencies to the Core so Chunk.cpp can use them
target_link_libraries(VoxelCore PUBLIC glad glfw imgui) 
set_strict_warnings(VoxelCore)

# --------------------------------------------------------
# 6. Main Executable
# --------------------------------------------------------
# Make sure your main file is listed here
add_executable(VoxelEngine "src/main.cpp") 
target_link_libraries(VoxelEngine PRIVATE VoxelCore)
set_strict_warnings(VoxelEngine)

# --------------------------------------------------------
# 5. Unit Tests
# --------------------------------------------------------
enable_testing()

add_executable(unit_tests 
    tests/test_main.cpp
    tests/test_physics.cpp
    tests/test_chunk.cpp
)

# Link the VoxelCore library (which contains Chunk.cpp) and GTest
target_link_libraries(unit_tests 
    PRIVATE
    VoxelCore 
    GTest::gtest
)
set_strict_warnings(unit_tests)

include(GoogleTest)
gtest_discover_tests(unit_tests)