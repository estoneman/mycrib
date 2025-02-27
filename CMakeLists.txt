cmake_minimum_required(VERSION 3.20)
project(mycrib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_STANDARD 11)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)

add_executable(mycrib mycrib.c)

target_include_directories(mycrib PRIVATE ${MICROHTTPD_INCLUDE_DIRS})
target_link_directories(mycrib PRIVATE ${MICROHTTPD_LIBRARY_DIRS})
target_link_libraries(mycrib PRIVATE ${MICROHTTPD_LIBRARIES})
