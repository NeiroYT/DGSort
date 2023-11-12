file(GLOB_RECURSE ALL_SOURCES_FILES *.cpp *.h)
add_library(app_libs STATIC ${ALL_SOURCES_FILES})