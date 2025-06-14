cmake_minimum_required(VERSION 3.20)
project(mycrib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JANSSON REQUIRED jansson)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(SANITIZE_OPTIONS -fsanitize=address,pointer-subtract,undefined)

set(COMMON_C_FLAGS -Wall -Wextra -Wmissing-field-initializers -pedantic)

macro(target_add_compile_options target)
  target_compile_options(
      ${target} PRIVATE ${COMMON_C_FLAGS} $<$<CONFIG:Release>:-Werror -O3>
                      $<$<CONFIG:Debug>:-g -O0 ${SANITIZE_OPTIONS}>)
endmacro()

add_executable(mycrib)

set(C_SOURCE ./src/mycrib.c ./src/route.c ./src/db.c ./src/handler.c
             ./src/util/mhd.c ./src/util/util.c)

target_sources(mycrib PRIVATE ${C_SOURCE})

target_include_directories(mycrib PRIVATE include/)
target_include_directories(mycrib PRIVATE ${MICROHTTPD_INCLUDE_DIRS})

target_link_libraries(mycrib PRIVATE ${MICROHTTPD_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${SQLITE3_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${JANSSON_LIBRARIES})

target_add_compile_options(mycrib)

target_link_options(mycrib PRIVATE ${SANITIZE_OPTIONS})

if(MYCRIB_TEST)
  add_executable(test_util)
  set(C_SOURCE_TEST ./test/test_util.c ./src/util/util.c)
  target_sources(test_util PRIVATE ${C_SOURCE_TEST})
  target_include_directories(test_util PRIVATE include/)
  target_link_options(test_util PRIVATE ${SANITIZE_OPTIONS})
  target_add_compile_options(test_util)
endif()
