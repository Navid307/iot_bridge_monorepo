find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED) # Include functions provided by PkgConfig module.

pkg_check_modules(DBUS REQUIRED dbus-1) # This calls pkgconfig with appropriate arguments

include_directories(${DBUS_INCLUDE_DIRS})

add_executable(server main.cpp)

target_link_libraries(server proto ${Protobuf_LIBRARIES} ${DBUS_LIBRARIES})

