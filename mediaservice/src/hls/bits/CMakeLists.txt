#
cmake_minimum_required(VERSION 2.8)

project(wzdbits)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(MYPREFIX ${PROJECT_SOURCE_DIR})
set(UTIL ${PROJECT_SOURCE_DIR}/libavutil)
#configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")

add_definitions(-DUINT64_C -D__STDC_CONSTANT_MACROS)
include_directories("${MYPREFIX}/libavutil")
include_directories("${MYPREFIX}/compat")
include_directories("${MYPREFIX}")

set(wzdbits_SRCS ${MYPREFIX}/golomb_.cpp	${MYPREFIX}/golomb.c ${UTIL}/avstring.c 	${UTIL}/bprint.c 	${UTIL}/error.c 	${UTIL}/intmath.c 	${UTIL}/log.c 	${UTIL}/log2_tab.c 	${UTIL}/mathematics.c ${UTIL}/mem.c ${UTIL}/rational.c )

option(USE_MYMATH "user libmpeg2demux library" ON)

add_library(wzdbits SHARED ${wzdbits_SRCS})
add_library(wzdbits-static ${wzdbits_SRCS})

#install(TARGETS mpeg2demuxTest DESTINATION bin)
#install(FILES wzdBits.h DESTINATION include)