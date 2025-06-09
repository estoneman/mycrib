cmake_minimum_required(VERSION 3.20)
project(mycrib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Werror -pedantic)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fsanitize=address")
set(CMAKE_C_FLAGS_RELEASE "-O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JANSSON REQUIRED jansson)

set(C_SOURCE
    ./src/mycrib.c
    ./src/route.c
    ./src/db.c
    ./src/handler.c
    ./src/util/mhd.c
    ./src/util/util.c)

if(MYCRIB_TEST)
    set(C_SOURCE_TEST ./test/test_util.c ./src/util/util.c)
    add_executable(test_util ${C_SOURCE_TEST})
    target_include_directories(test_util PRIVATE include/)
endif()

add_executable(mycrib ${C_SOURCE})
target_include_directories(mycrib PRIVATE include/)
target_include_directories(mycrib PRIVATE ${MICROHTTPD_INCLUDE_DIRS})

target_link_libraries(mycrib PRIVATE ${MICROHTTPD_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${SQLITE3_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${JANSSON_LIBRARIES})
