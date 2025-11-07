cmake_minimum_required(VERSION 3.21)

project(engine LANGUAGES CXX)

include(FetchContent)

file(GLOB_RECURSE SOURCES "source/*.cpp")

FetchContent_Declare(
    vulkanite
    GIT_REPOSITORY https://github.com/lucas-kuchel/vulkanite
    GIT_TAG master
)

FetchContent_MakeAvailable(vulkanite)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(magic_enum CONFIG REQUIRED)

add_executable(engine ${SOURCES})

target_include_directories(engine PRIVATE
    "include"
    ${vulkanite_SOURCE_DIR}
    ${Stb_INCLUDE_DIR}
    ${LUA_INCLUDE_DIR}
    ${SOL2_INCLUDE_DIRS}
)

target_link_libraries(engine PRIVATE
    vulkanite
    nlohmann_json::nlohmann_json
    magic_enum::magic_enum
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND MSVC)
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_CLANG_CL)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_CLANG)
endif()

if(MSVC)
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_MSVC)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GCC")
    target_compile_definitions(engine PRIVATE ENGINE_COMPILER_GCC)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(engine PRIVATE ENGINE_BUILD_TYPE_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(engine PRIVATE ENGINE_BUILD_TYPE_RELEASE)
endif()

if(APPLE)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_APPLE)
endif()

if(UNIX)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_UNIX)
elseif(WIN32)
    target_compile_definitions(engine PRIVATE ENGINE_PLATFORM_WIN32)
endif()
