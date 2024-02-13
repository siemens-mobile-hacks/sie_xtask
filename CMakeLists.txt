cmake_minimum_required(VERSION 3.28.2)

include(../sdk/config.cmake)
project(sieelf_sie_xtask C)

include_directories("../libsie/src/include")
link_libraries(crt crt_helper c gcc ft sie)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(sie_xtask ${SOURCES})
target_sdk_setup(sie_xtask ELKA)
target_link_directories(sie_xtask PUBLIC "../libsie/build/lib/ELKA")
target_compile_options(sie_xtask BEFORE PUBLIC -std=gnu99 -Wall -Werror)
