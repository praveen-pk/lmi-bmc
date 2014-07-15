cmake_minimum_required(VERSION 2.6)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LIB_SUFFIX "")
else(CMAKE_SIZEOF_VOID_P EQUAL 4)
    SET(LIB_SUFFIX 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

set (CMAKE_INSTALL_PREFIX /usr)

set(CMAKE_C_FLAGS " -pedantic -g -Wextra -Wno-unused-parameter -Wformat -Wparentheses -Wl,--no-undefined ${CMAKE_C_FLAGS}")

include(OpenLMIMacros RESULT_VARIABLE LMIMACROS)

if (${LMIMACROS} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "OpenLMIMacros.cmake not found, check if openlmi-providers(-devel) is installed")
endif (${LMIMACROS} STREQUAL "NOTFOUND")

find_package(CMPI REQUIRED)
find_package(KonkretCMPI REQUIRED)

add_subdirectory(src)
add_subdirectory(mof)


