cmake_minimum_required(VERSION 3.10)
project(sloflang)
set(CMAKE_CXX_STANDARD 20)

set(COMPILER_SOURCES
    compiler.cpp
)

set(BINARY_NAME sloflang)

add_executable(${BINARY_NAME} ${COMPILER_SOURCES})
include_directories(.)
add_custom_target(run
    COMMAND ${BINARY_NAME}
    DEPENDS ${BINARY_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(MSVC)
  target_compile_options(${BINARY_NAME} PRIVATE /W4 /WX)
else()
  target_compile_options(${BINARY_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()