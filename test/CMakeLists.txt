cmake_minimum_required(VERSION 3.10)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

set(XTDRAW_UNIT_TESTS_SOURCES
    src/main.cpp
    src/ansi_sequences.cpp
    src/utf8_encode_decode.cpp
)

# Create executable
add_executable(xtdraw_unit_tests ${XTDRAW_UNIT_TESTS_SOURCES})

target_include_directories(xtdraw_unit_tests
                           PUBLIC ${CMAKE_SOURCE_DIR}/app/include
                           PUBLIC ${CMAKE_SOURCE_DIR}/test/include
                       )

target_link_libraries(xtdraw_unit_tests
                      PRIVATE c
                      PRIVATE gtest
                      PRIVATE xtdraw_core)


