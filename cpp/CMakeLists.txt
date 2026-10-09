cmake_minimum_required(VERSION 3.20)
project(serpentine CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_executable(serpentine
    main.cpp
    snake_game.cpp
    ../third_party/miniz.c
)

target_include_directories(serpentine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../third_party
)

target_compile_options(serpentine PRIVATE /O2 /EHsc /MT)

target_link_libraries(serpentine
    winhttp
    gdiplus
    shell32
    advapi32
    ole32
    shlwapi
)