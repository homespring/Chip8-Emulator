cmake_minimum_required(VERSION 2.8.11)

set(PROJECT_NAME "Chip8_emulator")
project(${PROJECT_NAME})

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB SOURCES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp *.c)
file(GLOB HEADERS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.hpp *.h)

source_group(Headers FILES ${HEADERS})

find_package(SDL2 REQUIRED)

# Visual Leak Detector
find_path(VLD_INCLUDE_DIR vld.h $ENV{VLD_HOME}/include)
message("VLD_INCLUDE_DIR=${VLD_INCLUDE_DIR}")

find_library(VLD_LIBRARY NAMES vld
PATHS $ENV{VLD_HOME}/lib/Win64)
message("VLD_LIBRARY=${VLD_LIBRARY}")

set(ALLSOURCES ${SOURCES})
set(ALLHEADERS ${HEADERS})

include_directories(${VLD_INCLUDE_DIR})
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${VLD_LIBRARY} ${SDL2_LIBS})