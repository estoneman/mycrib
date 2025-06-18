cmake_minimum_required(VERSION 3.20)

set(MAIN "mycrib")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(SANITIZE_OPTIONS -fsanitize=address,pointer-subtract,undefined)
set(COMMON_C_FLAGS -Wall -Wextra -Wmissing-field-initializers -pedantic)

project(${MAIN})
add_executable(${MAIN})

message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_C_COMPILER clang)
    target_compile_definitions(${MAIN} PRIVATE -D_MACH=1)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_COMPILER gcc)
    target_compile_definitions(${MAIN} PRIVATE -D_LINUX=1)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JANSSON REQUIRED jansson)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

macro(target_add_compile_options target)
  target_compile_options(
      ${target} PRIVATE ${COMMON_C_FLAGS} $<$<CONFIG:Release>:-Werror -O3>
                      $<$<CONFIG:Debug>:-g -O0 ${SANITIZE_OPTIONS}>)
endmacro()

set(C_SOURCE ./src/mycrib.c ./src/route.c ./src/db.c ./src/handler.c
             ./src/util/mhd.c ./src/util/util.c)

target_sources(${MAIN} PRIVATE ${C_SOURCE})

target_include_directories(${MAIN} PRIVATE include/)
target_include_directories(${MAIN} PRIVATE ${MICROHTTPD_INCLUDE_DIRS})
target_include_directories(${MAIN} PRIVATE ${JANSSON_INCLUDE_DIRS})

target_link_directories(${MAIN} PRIVATE ${MICROHTTPD_LIBRARY_DIRS})
target_link_directories(${MAIN} PRIVATE ${JANSSON_LIBRARY_DIRS})

target_link_libraries(${MAIN} PRIVATE ${MICROHTTPD_LIBRARIES})
target_link_libraries(${MAIN} PRIVATE ${SQLITE3_LIBRARIES})
target_link_libraries(${MAIN} PRIVATE ${JANSSON_LIBRARIES})

target_add_compile_options(${MAIN})

target_link_options(${MAIN} PRIVATE ${SANITIZE_OPTIONS})

if(MYCRIB_TEST)
  add_executable(test_util)
  set(C_SOURCE_TEST ./test/test_util.c ./src/util/util.c)
  target_sources(test_util PRIVATE ${C_SOURCE_TEST})
  target_include_directories(test_util PRIVATE include/)
  target_link_options(test_util PRIVATE ${SANITIZE_OPTIONS})
  target_add_compile_options(test_util)
endif()
