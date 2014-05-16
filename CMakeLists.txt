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

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

if( WIN32 )
  add_definitions( -DPLATTFORM_WINDOWS )
  set (LIBS ${LIBS} Kernel32.dll)
  set(OS_PREFIX ${CMAKE_INSTALL_BINDIR}/..)
  #set(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_BINDIR})
else()
  set(OS_PREFIX /usr)
endif()

message("OS Prefix ${OS_PREFIX}")
message("CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR}")
message("CMAKE_LIBRARY_OUTPUT_PATH ${CMAKE_LIBRARY_OUTPUT_PATH}")

include_directories(
  ${OS_PREFIX}/include/gstreamer-1.0
  ${OS_PREFIX}/include/gstreamer-1.0/ges
  ${OS_PREFIX}/include/glib-2.0
  ${OS_PREFIX}/lib/glib-2.0/include
  ${OS_PREFIX}/include/json-glib-1.0
  ${CMAKE_SOURCE_DIR}/src
)

add_executable(ges-json src/ges-json.c src/ges-renderer.c)

target_link_libraries(ges-json ${LIBS} json-glib-1.0)

file(GLOB_RECURSE GLOB_TESTS tests/*.c)
foreach(TEST_PATH ${GLOB_TESTS})
    string(REGEX REPLACE "\\.c" "" TEST_NAME "${TEST_PATH}")
    string(REGEX REPLACE "^.*/([^/]*)\$" "\\1" TEST_NAME "${TEST_NAME}")
    add_executable(${TEST_NAME} ${TEST_PATH} src/ges-renderer.c)
    target_link_libraries(${TEST_NAME} ${LIBS})
endforeach(TEST_PATH)

#install
set(BINPERMISSIONS PERMISSIONS
	OWNER_READ OWNER_WRITE OWNER_EXECUTE
	GROUP_READ GROUP_EXECUTE
	WORLD_READ WORLD_EXECUTE)

if( WIN32 )
  set (INSTALL_BINARIES
	  ${CMAKE_SOURCE_DIR}/bin/ges-tests-format.exe
    ${CMAKE_SOURCE_DIR}/bin/ges-tests.exe
	  ${CMAKE_SOURCE_DIR}/bin/ges-minimal.exe
	  ${CMAKE_SOURCE_DIR}/bin/ges-json.exe
  )
else()
  set (INSTALL_BINARIES
	  ${CMAKE_SOURCE_DIR}/bin/ges-demo
	  ${CMAKE_SOURCE_DIR}/bin/ges-minimal
	  ${CMAKE_SOURCE_DIR}/bin/ges-json
  )
endif()

install(FILES ${INSTALL_BINARIES} DESTINATION "${OS_PREFIX}/bin" ${BINPERMISSIONS})
