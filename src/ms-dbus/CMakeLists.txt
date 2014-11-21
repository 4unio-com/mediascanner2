include_directories(..)
add_definitions(${MEDIASCANNER_CFLAGS} ${APPARMOR_CFLAGS})

add_library(ms-dbus STATIC
  qtservice.cc qtservice.h qtclient.cc qtclient.h
  comcanonicalmediascanner2.cc comcanonicalmediascanner2.h
)

qt5_use_modules(ms-dbus Core DBus)

target_link_libraries(ms-dbus
  mediascanner
  ${APPARMOR_LDFLAGS})
# Compile with -fPIC, since this code is linked into the QML plugin.
set_target_properties(ms-dbus PROPERTIES COMPILE_FLAGS "-fPIC")
set_target_properties(ms-dbus PROPERTIES AUTOMOC TRUE)

add_executable(mediascanner-dbus main.cc)
target_link_libraries(mediascanner-dbus mediascanner)
qt5_use_modules(mediascanner-dbus Core DBus)
set_target_properties(mediascanner-dbus PROPERTIES AUTOMOC TRUE)

target_link_libraries(mediascanner-dbus ms-dbus)
set_target_properties(mediascanner-dbus
  PROPERTIES OUTPUT_NAME "mediascanner-dbus-2.0")

install(
  TARGETS mediascanner-dbus
  RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/mediascanner-2.0
)

configure_file(
  com.canonical.MediaScanner2.service.in
  com.canonical.MediaScanner2.service)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/com.canonical.MediaScanner2.service
  DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
