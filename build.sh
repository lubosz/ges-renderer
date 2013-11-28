#export LIBRARY_PATH=/c/Users/Lubosz/cerbero/dist/windows_x86_64/lib/

cmake . \
  -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
  -DCMAKE_SYSTEM_NAME=Windows \
  -G"Unix Makefiles" \
  -DCMAKE_C_FLAGS="-Wall -g -O2 -DWINVER=0x0501  -Wno-error" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DWIN_LOCAL:BOOL=ON

#  -DCMAKE_FIND_ROOT_PATH=$CERBERO_PREFIX
#  -DCMAKE_CXX_FLAGS="-Wall -g -O2 -DWINVER=0x0501  -Wno-error "
#  -DLIB_SUFFIX=
#  -DCMAKE_INSTALL_PREFIX=/c/Users/Lubosz/cerbero/dist/windows_x86_64 \
#  -DCMAKE_INSTALL_BINDIR=/c/Users/Lubosz/cerbero/dist/windows_x86_64/bin \
#  -DCMAKE_LIBRARY_OUTPUT_PATH=/c/Users/Lubosz/cerbero/dist/windows_x86_64/lib \
#  -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
#  -DCMAKE_LIBRARY_PATH=/c/Users/Lubosz/cerbero/dist/windows_x86_64/lib/ \
#  -DCMAKE_FIND_ROOT_PATH=/c/Users/Lubosz/cerbero/dist/windows_x86_64/ \