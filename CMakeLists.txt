cmake_minimum_required(VERSION 3.10)
project(xtdraw)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

set(XTDRAW_SOURCES
    src/main.cpp
    src/xtdraw_app.cpp
    src/terminal_io.cpp
    src/board.cpp
    src/charset.cpp)

# Create executable
add_executable(${CMAKE_PROJECT_NAME} ${XTDRAW_SOURCES})

# Link necessary libraries (termios is part of libc on Linux)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE c)

