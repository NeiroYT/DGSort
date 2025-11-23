cmake_minimum_required(VERSION 3.25)

set(current_project "Neiro's Sorter")
project(${current_project})

add_subdirectory("thirdparty/SDL2")
add_subdirectory("thirdparty/SDL2_mixer")
add_subdirectory("thirdparty/SDL_ttf")
set(nfd_path "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nfd")
set(fi_path "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/FreeImage")
set(sdl_path "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2/include")
set(sdlm_path "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL2_mixer/include")
set(sdlttf_path "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/SDL_ttf")

include_directories("modules/include" "thirdparty" "${sdlm_path}" "${sdl_path}" "${sdlttf_path}")

add_subdirectory(application)
add_subdirectory(modules)


