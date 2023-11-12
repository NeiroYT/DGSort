set(app_name "Sorter")

add_executable(${app_name} DGSort.cpp)
target_link_libraries(${app_name} app_libs)
target_link_libraries(${app_name} "${fi_path}/FreeImage.lib")
target_link_libraries(${app_name} "${nfd_path}/nfd_d.lib")
target_link_libraries(${app_name} "${nfd_path}/nfd.lib")