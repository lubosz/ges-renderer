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

if( NOT WIN32 )
  add_cflag("-std=c99")
  add_cflag("-Werror")
  add_cflag("-Wall")
  add_cflag("-Wuninitialized")
  add_cflag("-Wpointer-arith")

  add_cflag("-Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wundef")
  add_cflag("-Wwrite-strings -Wformat-nonliteral -Wformat-security -Wold-style-definition")
  add_cflag("-Winit-self -Wmissing-include-dirs -Waddress -Waggregate-return -Wno-multichar")
  add_cflag("-Wnested-externs")
endif()

set(WIN_PREFIX C:/Users/bmonkey/cerbero/dist/windows_x86)

if( WIN32 )
  include_directories(
    ${WIN_PREFIX}/include/gstreamer-1.0
    ${WIN_PREFIX}/include/glib-2.0
    ${WIN_PREFIX}/lib/glib-2.0/include
    ${WIN_PREFIX}/include/json-glib-1.0
  )
else()
  include_directories(
    /usr/include/gstreamer-1.0
    /usr/include/glib-2.0
    /usr/lib/glib-2.0/include
    /usr/include/json-glib-1.0
  )
endif()

add_executable(ges-demo ges-demo.c ges-renderer.c)
add_executable(ges-minimal ges-minimal.c)
add_executable(ges-json ges-json.c)

set(LIBS gstreamer-1.0 ges-1.0 gobject-2.0 glib-2.0 gstpbutils-1.0)

if( WIN32 )
  set (LIBS ${LIBS} Kernel32.dll)
endif()

target_link_libraries(ges-demo ${LIBS})
target_link_libraries(ges-minimal ${LIBS})
target_link_libraries(ges-json ${LIBS} json-glib-1.0)

#install
set(BINPERMISSIONS PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)

if( WIN32 )
  set (INSTALL_BINARIES
	  ${CMAKE_SOURCE_DIR}/ges-demo.exe
	  ${CMAKE_SOURCE_DIR}/ges-minimal.exe
	  ${CMAKE_SOURCE_DIR}/ges-json.exe
  )
else()
  set (INSTALL_BINARIES
	  ${CMAKE_SOURCE_DIR}/ges-demo
	  ${CMAKE_SOURCE_DIR}/ges-minimal
	  ${CMAKE_SOURCE_DIR}/ges-json
  )
endif()

install(FILES ${INSTALL_BINARIES} DESTINATION "${WIN_PREFIX}/bin" ${BINPERMISSIONS})

