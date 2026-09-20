set(XTDRAW_APP_SOURCES
    src/main.cpp
    src/xtdraw_app.cpp
    src/board.cpp)

set(XTDRAW_CORE_SOURCES
    src/core/ansi_output.cpp
    src/core/ansi_sequences.cpp
    src/core/terminal_io.cpp
    src/core/charset.cpp
    src/core/colors.cpp
    src/core/utils.cpp
    src/formats/ansi.cpp)

add_library(xtdraw_core STATIC ${XTDRAW_CORE_SOURCES})
target_include_directories(xtdraw_core
                           PUBLIC ${CMAKE_SOURCE_DIR}/app/include)

add_executable(${CMAKE_PROJECT_NAME} ${XTDRAW_APP_SOURCES})
target_include_directories(${CMAKE_PROJECT_NAME}
                           PUBLIC ${CMAKE_SOURCE_DIR}/app/include)

# Link necessary libraries (termios is part of libc on Linux)
target_link_libraries(${CMAKE_PROJECT_NAME} 
    PRIVATE c
    PRIVATE xtdraw_core)

