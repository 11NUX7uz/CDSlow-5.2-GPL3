project(cdslow-help NONE)
cmake_minimum_required(VERSION 3.0)

find_program(CHMCMD_EXE chmcmd DOC "chmcmd executable from Free Pascal Compiler")

function(make_chm CHM_NAME CHM_SUBDIR CHM_HHP_IN)
    set(CHM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${CHM_SUBDIR}")
    string(REPLACE ".in" "" CHM_HHP "${CMAKE_BINARY_DIR}/${CHM_HHP_IN}")

    file(GLOB_RECURSE CHM_DEPS "${CHM_SUBDIR}/*")
    file(GLOB_RECURSE CHM_FILES RELATIVE ${CHM_DIR} "${CHM_DIR}/*.html" "${CHM_DIR}/*.gif")
    string(REPLACE ";" "\n" CHM_FILE_LIST "${CHM_FILES}")
    file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/${CHM_NAME}" CHM_OUTPUT)

    configure_file("${CHM_SUBDIR}/${CHM_HHP_IN}" ${CHM_HHP})

    add_custom_command(OUTPUT ${CHM_NAME}
        COMMAND ${CHMCMD_EXE} --no-html-scan ${CHM_HHP}
        DEPENDS ${CHM_DEPS}
        WORKING_DIRECTORY ${CHM_DIR}
        )
endfunction()

if(CHMCMD_EXE)
    message(STATUS "Using chmcmd from ${CHMCMD_EXE}")
    make_chm(cdslow-en.chm en cdslow-en.hhp.in)
    make_chm(cdslow-ru.chm ru cdslow-ru.hhp.in)
    add_custom_target(${PROJECT_NAME} ALL DEPENDS cdslow-en.chm cdslow-ru.chm)
else()
    message(WARNING "chmcmd NOT FOUND. Can't build help files.")
endif()

