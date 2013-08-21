cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-std=c99 -w") 

include_directories(
  /usr/include/gstreamer-1.0
  /usr/include/glib-2.0
  /usr/lib/glib-2.0/include/
)
add_executable(ges-demo ges-demo.c)

set(LIBS
gstreamer-1.0 ges-1.0 gobject-2.0 glib-2.0 gstpbutils-1.0
)

target_link_libraries(ges-demo ${LIBS})
