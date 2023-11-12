cmake_minimum_required(3.25)

set(current_project "Neiro's Sorter")
project(${current_project})

add_subdirectory("thirdparty/SDL")
add_subdirectory("thirdparty/nfd")
add_subdirectory("thirdparty/FreeImage")
set(filib_path "thirdparty/FreeImage/FreeImage.lib")
set(nfd_path "thirdparty/nfd/nfd.lib")
set(nfdd_path "thirdparty/nfd/nfd_d.lib")

include_directories("modules/include")
add_subdirectory(application)
add_subdirectory(modules)


