cmake_minimum_required(VERSION 3.20)
project(mycrib)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Werror -pedantic)

find_package(PkgConfig REQUIRED)
pkg_check_modules(MICROHTTPD REQUIRED libmicrohttpd)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)
pkg_check_modules(JANSSON REQUIRED jansson)

set(C_SOURCE src/mycrib.c src/route.c src/db.c src/handler.c)
add_executable(mycrib ${C_SOURCE})
target_include_directories(mycrib PRIVATE include/)

target_include_directories(mycrib PRIVATE ${MICROHTTPD_INCLUDE_DIRS})

target_link_libraries(mycrib PRIVATE ${MICROHTTPD_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${SQLITE3_LIBRARIES})
target_link_libraries(mycrib PRIVATE ${JANSSON_LIBRARIES})
