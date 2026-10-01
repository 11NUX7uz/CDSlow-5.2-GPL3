project(cdslow_exe C RC)
cmake_minimum_required(VERSION 3.0)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic -Wall -Wextra")
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} --codepage=65001")
endif(CMAKE_COMPILER_IS_GNUCC)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} /c 65001")
    set(XLINK "/MANIFEST:NO")
endif(MSVC)

string(TIMESTAMP BUILD_ID "[build %Y-%m-%d %H:%M:%S]")
configure_file(build.h.in build.h)
include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(libdlg)
include_directories(libdlg)

add_subdirectory(liblog)
include_directories(liblog)

add_subdirectory(libuniscsi)
include_directories(libuniscsi)

add_subdirectory(libmd5)
include_directories(libmd5)

add_subdirectory(src)

set(EXE_SRC
src/main.c
resource.rc
)

set(EXE_LIBS cdslow dlglib loglib uniscsi md5_crypt version comctl32)

add_executable(cdslow-en ${EXE_SRC})
set_property(TARGET cdslow-en APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lang/en")
target_link_libraries(cdslow-en ${EXE_LIBS})

add_executable(cdslow-ru ${EXE_SRC})
set_property(TARGET cdslow-ru APPEND PROPERTY INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/lang/ru")
target_link_libraries(cdslow-ru ${EXE_LIBS})

set_target_properties(cdslow-en cdslow-ru PROPERTIES LINK_FLAGS "${CMAKE_CREATE_WIN32_EXE} ${XLINK}")

if(CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT MSVC)
    add_custom_command(TARGET cdslow-en POST_BUILD COMMAND ${CMAKE_STRIP} "cdslow-en${CMAKE_EXECUTABLE_SUFFIX}")
    add_custom_command(TARGET cdslow-ru POST_BUILD COMMAND ${CMAKE_STRIP} "cdslow-ru${CMAKE_EXECUTABLE_SUFFIX}")
endif()

add_subdirectory(help)
