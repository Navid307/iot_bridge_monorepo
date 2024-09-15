project(main)

cmake_minimum_required(VERSION 3.22.0)

find_package(PkgConfig REQUIRED)

# pkg_search_module(GATTLIB REQUIRED gattlib)
pkg_search_module(PCRE REQUIRED libpcre)

add_definitions( -DGATTLIB_LOG_LEVEL=3 )

add_executable(main main.c)
target_link_libraries(main ${PCRE_LIBRARIES} pthread /usr/local/lib/libgattlib.so)



# add this to the README file: sudo make install
# -- Install configuration: "Debug"
# -- Installing: /usr/local/lib/libgattlib.so
# -- Installing: /usr/local/include/gattlib.h
# -- Installing: /usr/local/lib/pkgconfig/gattlib.pc