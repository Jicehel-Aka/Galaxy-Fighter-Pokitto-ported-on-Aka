cmake_minimum_required(VERSION 3.16)
project(GalaxyFighter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(ANDROID)
    if(NOT SDL2_SOURCE_DIR)
        message(FATAL_ERROR "Set SDL2_SOURCE_DIR to an SDL2 source checkout for Android builds")
    endif()
    add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/SDL2")
else()
    find_package(SDL2 CONFIG REQUIRED)
endif()

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../main/game/*.cpp")
if(ANDROID)
    add_library(galaxy-fighter SHARED
        src/main.cpp
        src/platform.cpp
        ${GAME_SOURCES}
    )
else()
    add_executable(galaxy-fighter
        src/main.cpp
        src/platform.cpp
        ${GAME_SOURCES}
    )
endif()
target_include_directories(galaxy-fighter PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../main"
    "${CMAKE_CURRENT_LIST_DIR}/../components/pokitto_compat/include"
    "${CMAKE_CURRENT_LIST_DIR}/../components/aka_runtime/include"
    "${CMAKE_CURRENT_LIST_DIR}/../components/gamebuino/include_lib"
    "${CMAKE_CURRENT_LIST_DIR}/../components/gamebuino/include_ll"
)
target_compile_definitions(galaxy-fighter PRIVATE INCLUDE_SOUND_FROM_SD INCLUDE_SOUND)
if(WIN32 AND TARGET SDL2::SDL2main)
    target_link_libraries(galaxy-fighter PRIVATE SDL2::SDL2main)
endif()
if(TARGET SDL2::SDL2)
    target_link_libraries(galaxy-fighter PRIVATE SDL2::SDL2)
else()
    target_link_libraries(galaxy-fighter PRIVATE SDL2::SDL2-static)
endif()

if(ANDROID)
    target_link_libraries(galaxy-fighter PRIVATE android log)
    set_target_properties(galaxy-fighter PROPERTIES
        OUTPUT_NAME main
    )
else()
    file(COPY "${CMAKE_CURRENT_LIST_DIR}/../sdcard_files/galaxy/music"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/galaxy")
endif()
