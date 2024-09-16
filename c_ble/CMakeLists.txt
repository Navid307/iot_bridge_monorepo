cmake_minimum_required(VERSION 3.22.0)

project(main)

find_package(PkgConfig REQUIRED)

# pkg_search_module(GATTLIB REQUIRED gattlib)
pkg_search_module(PCRE REQUIRED libpcre)

add_definitions( -DGATTLIB_LOG_LEVEL=3 )

add_executable(main main.c nus.c nus.h)
target_link_libraries(main ${PCRE_LIBRARIES} pthread /usr/local/lib/libgattlib.so)
