AUTOMAKE_OPTIONS=foreign
AM_CPPFLAGS=$(DEPS_CFLAGS) -std=gnu++11
mpegtest_LDADD=$(DEPS_LIBS)
libmpeg2_1_0_la_LIBADD=$(DEPS_CFLAGS)
libmpeg2_1_0_la_CPPFLAGS=$(DEPS_LIBS)
library_includedir=$(includedir)
library_include_HEADERS=bbits.h TsMuxer.h
ACLOCAL_AMFLAGS=-I m4
bin_PROGRAMS=mpegtest
lib_LTLIBRARIES=libmpeg2-1.0.la
pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=mpeg2.pc
mpegtest_SOURCES=BitWriter.h bitstreamfilter.cpp bitstreamfilter.h BitWriter.cpp main.cpp MemoryCache.cpp MemoryCache.h stdafx.h Stdafx.cpp TsMuxer.cpp TsMuxer.h 
libmpeg2_1_0_la_SOURCES=BitWriter.h bitstreamfilter.cpp bitstreamfilter.h BitWriter.cpp MemoryCache.cpp MemoryCache.h stdafx.h Stdafx.cpp TsMuxer.cpp TsMuxer.h 

