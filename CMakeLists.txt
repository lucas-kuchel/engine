cmake_minimum_required(VERSION 3.21)

project(engine LANGUAGES CXX)

file(GLOB_RECURSE SOURCES "source/*.cpp")

find_package(glfw3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(Lua REQUIRED)

find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")

add_executable(engine ${SOURCES})

target_include_directories(engine PRIVATE
    "include"
    ${Stb_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${SOL2_INCLUDE_DIRS}
)

target_link_libraries(engine PRIVATE
    glfw
    nlohmann_json::nlohmann_json
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    ${LUA_LIBRARIES}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(engine PRIVATE
        $<$<CONFIG:Release>:-O3 -flto -fomit-frame-pointer>
        -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor
        -Wcast-align -Wunused -Wconversion -Wsign-conversion
        -Wzero-as-null-pointer-constant
        -Wnull-dereference -Wdouble-promotion -Wformat=2 -Werror
    )
elseif(MSVC)
    message(WARNING "MSVC has not been tested with this codebase. Use at your own risk. For a more stable experience, consider using gcc, clang++ or clang-cl")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PRIVATE DEBUG_BUILD_TYPE)
endif()

if(APPLE)
    target_compile_definitions(engine PRIVATE PLATFORM_APPLE)
endif()
