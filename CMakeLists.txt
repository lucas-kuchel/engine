cmake_minimum_required(VERSION 3.21)

project(engine LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

find_package(glfw3 CONFIG REQUIRED)
find_package(VulkanHeaders CONFIG REQUIRED)

add_executable(engine ${SOURCES})

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE
        $<$<CONFIG:Release>:-O3 -flto -fomit-frame-pointer>
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
        -Wold-style-cast -Wcast-align -Wunused -Wconversion -Wsign-conversion
        -Wzero-as-null-pointer-constant -Wduplicated-cond -Wduplicated-branches
        -Wlogical-op -Wnull-dereference -Wdouble-promotion -Wformat=2 -Werror
    )
elseif(MSVC)
    target_compile_options(engine PRIVATE
        $<$<CONFIG:Release>:/O2 /GL /favor:INTEL64 /Ot /fp:fast>
        /W4 /permissive- /WX
    )
endif()

target_link_libraries(engine PRIVATE glfw Vulkan::Headers)

target_include_directories(engine PRIVATE "include")

if(APPLE)
    find_library(MOLTENVK_LIB MoltenVK REQUIRED)
    target_link_libraries(engine PRIVATE ${MOLTENVK_LIB})
    target_compile_definitions(engine PRIVATE PLATFORM_APPLE)
else()
    find_package(Vulkan REQUIRED)
    target_link_libraries(engine PRIVATE Vulkan::Vulkan)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PRIVATE DEBUG_BUILD_TYPE)
endif()
