cmake_minimum_required(VERSION 2.6)

set (LMIBMC_VERSION_MAJOR 0)
set (LMIBMC_VERSION_MINOR 5)
set (LMIBMC_VERSION_REVISION 2)
set (LMIBMC_VERSION "${LMIBMC_VERSION_MAJOR}.${LMIBMC_VERSION_MINOR}.${LMIBMC_VERSION_REVISION}")

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(LIB_SUFFIX "")
else(CMAKE_SIZEOF_VOID_P EQUAL 4)
    SET(LIB_SUFFIX 64)
endif(CMAKE_SIZEOF_VOID_P EQUAL 4)

set (CMAKE_INSTALL_PREFIX /usr)
set (DOCDIR "share/doc/openlmi-bmc/" )

add_definitions(-D_XOPEN_SOURCE=500 -D_GNU_SOURCE)
set(CMAKE_C_FLAGS " -pedantic -g -Wextra -Wno-unused-parameter -Wformat -Wparentheses -Wl,--no-undefined ${CMAKE_C_FLAGS}")

find_package(PkgConfig)
pkg_check_modules (GLIB   glib-2.0)

include(OpenLMIMacros RESULT_VARIABLE LMIMACROS)

if (${LMIMACROS} STREQUAL "NOTFOUND")
    message(FATAL_ERROR "OpenLMIMacros.cmake not found, check if openlmi-providers(-devel) is installed")
endif (${LMIMACROS} STREQUAL "NOTFOUND")

install( FILES COPYING DESTINATION ${DOCDIR} )
find_package(CMPI REQUIRED)
find_package(KonkretCMPI REQUIRED)

add_subdirectory(src)
add_subdirectory(mof)
add_subdirectory(pkg)
