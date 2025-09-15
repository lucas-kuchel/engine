cmake_minimum_required(VERSION 3.21)

project(engine LANGUAGES CXX)

file(GLOB_RECURSE CPP_SOURCES "source/*.cpp")

add_executable(engine)

if(UNIX)
    target_compile_definitions(engine PRIVATE PLATFORM_UNIX)
endif()

if(APPLE)
    target_compile_definitions(engine PRIVATE PLATFORM_APPLE)
elseif(WIN32)
    target_compile_definitions(engine PRIVATE PLATFORM_WINDOWS)
endif()

if (UNIX AND NOT APPLE)
    find_package(VulkanHeaders CONFIG REQUIRED)
    find_package(Vulkan REQUIRED)
    target_link_libraries(engine PRIVATE Vulkan::Headers Vulkan::Vulkan)
endif()

if(APPLE)
    find_library(METAL_FRAMEWORK Metal)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore)
    find_package(spirv_cross_msl CONFIG REQUIRED)
    file(GLOB_RECURSE MM_SOURCES "source/*.mm")
    target_sources(engine PRIVATE ${MM_SOURCES})
    target_link_libraries(engine PRIVATE ${METAL_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} spirv-cross-msl)
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)

target_compile_options(engine PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -O0>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /Od>
)

target_link_libraries(engine PRIVATE 
    glfw
    glm::glm
    spirv-cross-core
)

target_sources(engine PRIVATE ${CPP_SOURCES})

target_include_directories(engine PRIVATE "include")