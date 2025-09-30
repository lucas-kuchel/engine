cmake_minimum_required(VERSION 3.21)

project(engine LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "source/*.cpp")

find_package(glfw3 CONFIG REQUIRED)

add_executable(engine ${SOURCES})

target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

target_link_libraries(engine PRIVATE glfw)

# For GCC/Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow             # Warn if a variable shadows another
        -Wnon-virtual-dtor    # Warn if a polymorphic class lacks virtual dtor
        -Wold-style-cast
        -Wcast-align
        -Wunused
        -Wconversion
        -Wsign-conversion
        -Wzero-as-null-pointer-constant
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
        -Wnull-dereference
        -Wdouble-promotion
        -Wformat=2
        -Werror              # Treat all warnings as errors
    )
endif()

# For MSVC
if(MSVC)
    target_compile_options(engine PRIVATE
        /W4           # Maximum warnings
        /permissive-  # Strict ISO compliance
        /WX           # Treat warnings as errors
    )
endif()


if (NOT APPLE)
    find_package(VulkanHeaders CONFIG REQUIRED)
    find_package(Vulkan REQUIRED)

    target_link_libraries(engine PRIVATE Vulkan::Vulkan Vulkan::Headers)
else()
    find_library(METAL_FRAMEWORK Metal)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)

    find_package(spirv_cross_core CONFIG REQUIRED)
    find_package(spirv_cross_glsl CONFIG REQUIRED)
    find_package(spirv_cross_msl CONFIG REQUIRED)

    file(GLOB_RECURSE MM_SOURCES "source/*.mm")

    target_sources(engine PRIVATE ${MM_SOURCES})

    target_link_libraries(engine PRIVATE ${METAL_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} spirv-cross-core spirv-cross-glsl spirv-cross-msl)

    target_compile_definitions(engine PRIVATE PLATFORM_APPLE)
endif()

target_include_directories(engine PRIVATE "include")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PRIVATE DEBUG_BUILD_TYPE)
endif()