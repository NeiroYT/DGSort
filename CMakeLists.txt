cmake_minimum_required(VERSION 3.25)

set(current_project "Neiro's Sorter")
project(${current_project})

add_subdirectory("thirdparty/SDL")
set(nfd_path "thirdparty/nfd")
set(fi_path "thirdparty/FreeImage")
set(sdl_path "thirdparty/SDL/include")
set(sdlm_path "thirdparty/SDL_mixer/include")

include_directories("modules/include" "thirdparty" "${sdlm_path}" "${sdl_path}")

add_subdirectory(application)
add_subdirectory(modules)


