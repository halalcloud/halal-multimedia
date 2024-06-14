#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../inc -I/opt/intel/mediasdk/include -Isample_common/include -I/usr/include/libdrm/ -Iinclude
CFLAGS = -Wall -std=c++11 -fPIC -fvisibility=hidden -fexceptions -DUNIX -D__USE_LARGEFILE64 -DLIBVA_SUPPORT -DLIBVA_DRM_SUPPORT -DLIBVA_X11_SUPPOR -DBOOST_LOG_DYN_LINK
RESINC = 
LIBDIR = -L../IntelCodec -L/opt/intel/mediasdk/lib/lin_x64/
LIB = libsample_common.a -ldl libmfx.a
LDFLAGS = -Wl,--rpath=./lib/

INC_DEBUG = $(INC) -Iinclude -I../IntelCodec
CFLAGS_DEBUG = $(CFLAGS) -g
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = ../obj/Debug/IntelCodec
DEP_DEBUG = 
OUT_DEBUG = ../bin/Debug/IntelCodec.so

INC_RELEASE = $(INC) -Iinclude -I../IntelCodec
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = ../obj/Release/IntelCodec
DEP_RELEASE = 
OUT_RELEASE = ../bin/Release/IntelCodec.so

OBJ_DEBUG = $(OBJDIR_DEBUG)/IntelVideoDecoder.o $(OBJDIR_DEBUG)/IntelVideoEncoder.o $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/pipeline_decode.o $(OBJDIR_DEBUG)/pipeline_encode.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/IntelVideoDecoder.o $(OBJDIR_RELEASE)/IntelVideoEncoder.o $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/pipeline_decode.o $(OBJDIR_RELEASE)/pipeline_encode.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d ../bin/Debug || mkdir -p ../bin/Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) -shared $(LIBDIR_DEBUG) $(OBJ_DEBUG)  -o $(OUT_DEBUG) $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/IntelVideoDecoder.o: IntelVideoDecoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c IntelVideoDecoder.cpp -o $(OBJDIR_DEBUG)/IntelVideoDecoder.o

$(OBJDIR_DEBUG)/IntelVideoEncoder.o: IntelVideoEncoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c IntelVideoEncoder.cpp -o $(OBJDIR_DEBUG)/IntelVideoEncoder.o

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

$(OBJDIR_DEBUG)/pipeline_decode.o: pipeline_decode.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c pipeline_decode.cpp -o $(OBJDIR_DEBUG)/pipeline_decode.o

$(OBJDIR_DEBUG)/pipeline_encode.o: pipeline_encode.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c pipeline_encode.cpp -o $(OBJDIR_DEBUG)/pipeline_encode.o

clean_debug: 
	rm -f $(OBJ_DEBUG) $(OUT_DEBUG)
	rm -rf ../bin/Debug
	rm -rf $(OBJDIR_DEBUG)

before_release: 
	test -d ../bin/Release || mkdir -p ../bin/Release
	test -d $(OBJDIR_RELEASE) || mkdir -p $(OBJDIR_RELEASE)

after_release: 

release: before_release out_release after_release

out_release: before_release $(OBJ_RELEASE) $(DEP_RELEASE)
	$(LD) -shared $(LIBDIR_RELEASE) $(OBJ_RELEASE)  -o $(OUT_RELEASE) $(LDFLAGS_RELEASE) $(LIB_RELEASE)

$(OBJDIR_RELEASE)/IntelVideoDecoder.o: IntelVideoDecoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c IntelVideoDecoder.cpp -o $(OBJDIR_RELEASE)/IntelVideoDecoder.o

$(OBJDIR_RELEASE)/IntelVideoEncoder.o: IntelVideoEncoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c IntelVideoEncoder.cpp -o $(OBJDIR_RELEASE)/IntelVideoEncoder.o

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/pipeline_decode.o: pipeline_decode.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c pipeline_decode.cpp -o $(OBJDIR_RELEASE)/pipeline_decode.o

$(OBJDIR_RELEASE)/pipeline_encode.o: pipeline_encode.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c pipeline_encode.cpp -o $(OBJDIR_RELEASE)/pipeline_encode.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf ../bin/Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release
