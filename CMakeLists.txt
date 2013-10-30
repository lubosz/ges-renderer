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

add_cflag("-std=c99")

if( NOT WIN32 )
  add_cflag("-Werror")
  add_cflag("-Wall")
  add_cflag("-Wuninitialized")
  add_cflag("-Wpointer-arith")

  add_cflag("-Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wundef")
  add_cflag("-Wwrite-strings -Wformat-nonliteral -Wformat-security -Wold-style-definition")
  add_cflag("-Winit-self -Wmissing-include-dirs -Waddress -Waggregate-return -Wno-multichar")
  add_cflag("-Wnested-externs")
endif()

set(LIBS gstreamer-1.0 ges-1.0 gobject-2.0 glib-2.0 gstpbutils-1.0)

if( WIN32 )
  add_definitions( -DPLATTFORM_WINDOWS )
  set (LIBS ${LIBS} Kernel32.dll)
  set(OS_PREFIX C:/Users/bmonkey/cerbero/dist/windows_x86)
else()
  set(OS_PREFIX /usr)
endif()

include_directories(
  ${OS_PREFIX}/include/gstreamer-1.0
  ${OS_PREFIX}/include/glib-2.0
  ${OS_PREFIX}/lib/glib-2.0/include
  ${OS_PREFIX}/include/json-glib-1.0
)

add_executable(ges-demo ges-demo.c ges-renderer.c)
add_executable(ges-minimal ges-minimal.c)
add_executable(ges-json ges-json.c ges-renderer.c)
add_executable(ges-minimal-transition ges-minimal-transition.c)

target_link_libraries(ges-demo ${LIBS})
target_link_libraries(ges-minimal ${LIBS})
target_link_libraries(ges-json ${LIBS} json-glib-1.0)
target_link_libraries(ges-minimal-transition ${LIBS})

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
    ${CMAKE_SOURCE_DIR}/ges-minimal-transition.exe
  )
else()
  set (INSTALL_BINARIES
	  ${CMAKE_SOURCE_DIR}/ges-demo
	  ${CMAKE_SOURCE_DIR}/ges-minimal
	  ${CMAKE_SOURCE_DIR}/ges-json
    ${CMAKE_SOURCE_DIR}/ges-minimal-transition
  )
endif()

install(FILES ${INSTALL_BINARIES} DESTINATION "${OS_PREFIX}/bin" ${BINPERMISSIONS})
