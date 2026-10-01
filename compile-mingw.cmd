set BUILD_TYPE=Release
set BUILD_DIR=build
set PATH=%PATH%;c:\mingw32\bin

mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%" || goto :EOF

cmake -G "MinGW Makefiles" -D CMAKE_BUILD_TYPE=%BUILD_TYPE% .. && cmake --build .

popd
