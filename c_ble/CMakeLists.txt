cmake_minimum_required(VERSION 3.22.0)

project(main)

find_package(PkgConfig REQUIRED)

# pkg_search_module(GATTLIB REQUIRED gattlib)
pkg_search_module(PCRE REQUIRED libpcre)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO REQUIRED gio-2.0)
pkg_search_module(DBUS REQUIRED dbus-1)
pkg_search_module(DBUSGLIB REQUIRED dbus-glib-1)

add_definitions( -DGATTLIB_LOG_LEVEL=3 )

include_directories(${GLIB_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS} ${DBUSGLIB_INCLUDE_DIRS})

add_executable(main main.c nus.c nus.h dbus.c dbus.h)
target_link_libraries(main ${PCRE_LIBRARIES} ${GLIB_LIBRARIES} ${DBUS_LIBRARIES} ${GLIBDBUS_LIBRARIES} ${GIO_LIBRARIES} pthread /usr/local/lib/libgattlib.so)
