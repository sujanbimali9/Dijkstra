cmake_minimum_required(VERSION 3.10)

project(DIJKSTRA)

set(CMAKE_PREFIX_PATH "/home/sujan/dev-tools/vcpkg/installed/X64-linux/share" ${CMAKE_PREFIX_PATH})


add_executable(DIJKSTRA src/main.cpp)
# add_compile_options("-g")

find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
find_package(sdl2-gfx CONFIG REQUIRED)

target_link_libraries(DIJKSTRA
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
target_link_libraries(DIJKSTRA PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
target_link_libraries(DIJKSTRA PRIVATE SDL2::SDL2_gfx)
