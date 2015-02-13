pkg_check_modules(DBUSCPP dbus-cpp REQUIRED)

add_library(
  mediascanner-dbus-static OBJECT
  d-bus/dbus-codec.cc
  d-bus/service-stub.cc
)

set_target_properties(
  mediascanner-dbus-static
  PROPERTIES
    LINK_FLAGS "-Wl,--export-all-symbols"
    COMPILE_FLAGS -fPIC
)

add_library(mediascanner SHARED
  MediaFile.cc
  MediaFileBuilder.cc
  MediaFilePrivate.cc
  Filter.cc
  Album.cc
  MediaStore.cc
  MediaStoreBase.cc
  utils.cc
  mozilla/fts3_porter.c
  mozilla/Normalize.c
  $<TARGET_OBJECTS:mediascanner-dbus-static>
)

set(symbol_map "${CMAKE_CURRENT_SOURCE_DIR}/mediascanner-2.0.map")
set_target_properties(mediascanner PROPERTIES
    LINK_FLAGS "${ldflags} -Wl,--version-script,${symbol_map}")
set_target_properties(mediascanner PROPERTIES LINK_DEPENDS ${symbol_map})

add_definitions(${DBUSCPP_CFLAGS} ${MEDIASCANNER_CFLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(mediascanner ${MEDIASCANNER_LIBRARIES} ${DBUSCPP_LDFLAGS})

set_target_properties(mediascanner PROPERTIES
  OUTPUT_NAME "mediascanner-2.0"
  SOVERSION ${MEDIASCANNER_SOVERSION}
  VERSION ${MEDIASCANNER_LIBVERSION}
)

install(
  TARGETS mediascanner
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  Album.hh
  Filter.hh
  MediaFile.hh
  MediaFileBuilder.hh
  MediaStore.hh
  MediaStoreBase.hh
  scannercore.hh
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/mediascanner-2.0/mediascanner"
)
