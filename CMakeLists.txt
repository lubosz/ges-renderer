cmake_minimum_required(VERSION 2.8)

function(add_cflag FLAG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}" PARENT_SCOPE)
endfunction(add_cflag)


if(CMAKE_BUILD_TYPE STREQUAL Debug)
    #Debug
    message("On Debug Build")
    add_cflag("-O0") 
    add_cflag("-g3") 
else()
    #Release
    message("On Release Build")
    add_cflag("-O3")
endif()

#set(CMAKE_C_FLAGS "-std=c99 -w") 

add_cflag("-Werror")
add_cflag("-Wall")

#add_cflag("-Wall")
#add_cflag("-Wextra")
#add_cflag("-Wuninitialized")
#add_cflag("-Wold-style-cast")
#add_cflag("-Woverloaded-virtual")
#add_cflag("-Wformat=2")
#add_cflag("-Wundef")
#add_cflag("-Wpointer-arith")
#add_cflag("-Wcast-qual")
#add_cflag("-Wlogical-op")
#add_cflag("-Woverflow")
#add_cflag("-Wmissing-declarations")
#add_cflag("-Wredundant-decls")


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
