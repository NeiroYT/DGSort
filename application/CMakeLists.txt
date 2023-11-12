set(app_name "Sorter")

add_executable(${app_name} DGSort.cpp)
target_link_libraries(${app_name} app_libs)
target_link_libraries(${app_name} SDL2)
target_link_libraries(${app_name} SDL2_mixer)
target_link_libraries(${app_name} "${fi_path}/FreeImage.lib")
target_link_libraries(${app_name} "${nfd_path}/nfd_d.lib")
target_link_libraries(${app_name} "${nfd_path}/nfd.lib")

if (NOT $<CONFIG:Debug>)
	target_link_options(${app_name} PRIVATE "/SUBSYSTEM:WINDOWS")
	target_link_options(${app_name} PRIVATE /ENTRY:\"mainCRTStartup\")
endif (NOT $<CONFIG:Debug>)