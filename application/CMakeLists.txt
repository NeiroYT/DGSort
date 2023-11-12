set(app_name "Sorter")

add_executable(${app_name} DGSort.cpp)
target_link_libraries(app_name app_libs)
target_link_libraries(app_name filib_path)
target_link_libraries(app_name nfd_path)
target_link_libraries(app_name nfdd_path)