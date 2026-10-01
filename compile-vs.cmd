set BUILD_TYPE=Release
set BUILD_DIR=build

mkdir "%BUILD_DIR%"
pushd "%BUILD_DIR%" || goto :EOF

cmake .. && cmake --build . --config %BUILD_TYPE%

popd
