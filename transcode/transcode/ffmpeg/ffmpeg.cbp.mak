#------------------------------------------------------------------------------#
# This makefile was generated by 'cbp2make' tool rev.147                       #
#------------------------------------------------------------------------------#


WORKDIR = `pwd`

CC = gcc
CXX = g++
AR = ar
LD = g++
WINDRES = windres

INC = -I../inc -I../../ThirdParty/ffSDK/include -I../../ThirdParty/ffSDK/src -I../src
CFLAGS = -Wall -fPIC -fvisibility=hidden -fexceptions -pthread -lz -lbz2
RESINC = 
LIBDIR = -L../../ThirdParty/ffSDK/lib
LIB = 
LDFLAGS = -Wl,--rpath=./lib/ -lavcodec -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

INC_DEBUG = $(INC) -INetService -I../ffmpeg
CFLAGS_DEBUG = $(CFLAGS) -g -D_DEBUG
RESINC_DEBUG = $(RESINC)
RCFLAGS_DEBUG = $(RCFLAGS)
LIBDIR_DEBUG = $(LIBDIR)
LIB_DEBUG = $(LIB)
LDFLAGS_DEBUG = $(LDFLAGS)
OBJDIR_DEBUG = ../obj/Debug/ffmpeg
DEP_DEBUG = 
OUT_DEBUG = ../bin/Debug/ffmpeg.so

INC_RELEASE = $(INC) -INetService -I../ffmpeg
CFLAGS_RELEASE = $(CFLAGS) -O2
RESINC_RELEASE = $(RESINC)
RCFLAGS_RELEASE = $(RCFLAGS)
LIBDIR_RELEASE = $(LIBDIR)
LIB_RELEASE = $(LIB)
LDFLAGS_RELEASE = $(LDFLAGS) -s
OBJDIR_RELEASE = ../obj/Release/ffmpeg
DEP_RELEASE = 
OUT_RELEASE = ../bin/Release/ffmpeg.so

OBJ_DEBUG = $(OBJDIR_DEBUG)/main.o $(OBJDIR_DEBUG)/MediaType.o $(OBJDIR_DEBUG)/FFmpegVideoScale.o $(OBJDIR_DEBUG)/FFmpegVideoEncoder.o $(OBJDIR_DEBUG)/FFmpegVideoDecoder.o $(OBJDIR_DEBUG)/FFmpegAudioDecoder.o $(OBJDIR_DEBUG)/FFmpegOSD.o $(OBJDIR_DEBUG)/FFmpegMuxer.o $(OBJDIR_DEBUG)/FFmpegDemuxer.o $(OBJDIR_DEBUG)/FFmpegAudioResample.o $(OBJDIR_DEBUG)/FFmpegAudioEncoder.o

OBJ_RELEASE = $(OBJDIR_RELEASE)/main.o $(OBJDIR_RELEASE)/MediaType.o $(OBJDIR_RELEASE)/FFmpegVideoScale.o $(OBJDIR_RELEASE)/FFmpegVideoEncoder.o $(OBJDIR_RELEASE)/FFmpegVideoDecoder.o $(OBJDIR_RELEASE)/FFmpegAudioDecoder.o $(OBJDIR_RELEASE)/FFmpegOSD.o $(OBJDIR_RELEASE)/FFmpegMuxer.o $(OBJDIR_RELEASE)/FFmpegDemuxer.o $(OBJDIR_RELEASE)/FFmpegAudioResample.o $(OBJDIR_RELEASE)/FFmpegAudioEncoder.o

all: debug release

clean: clean_debug clean_release

before_debug: 
	test -d ../bin/Debug || mkdir -p ../bin/Debug
	test -d $(OBJDIR_DEBUG) || mkdir -p $(OBJDIR_DEBUG)

after_debug: 

debug: before_debug out_debug after_debug

out_debug: before_debug $(OBJ_DEBUG) $(DEP_DEBUG)
	$(LD) -shared $(LIBDIR_DEBUG) $(OBJ_DEBUG)  -o $(OUT_DEBUG) $(LDFLAGS_DEBUG) $(LIB_DEBUG)

$(OBJDIR_DEBUG)/main.o: main.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c main.cpp -o $(OBJDIR_DEBUG)/main.o

$(OBJDIR_DEBUG)/MediaType.o: MediaType.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c MediaType.cpp -o $(OBJDIR_DEBUG)/MediaType.o

$(OBJDIR_DEBUG)/FFmpegVideoScale.o: FFmpegVideoScale.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegVideoScale.cpp -o $(OBJDIR_DEBUG)/FFmpegVideoScale.o

$(OBJDIR_DEBUG)/FFmpegVideoEncoder.o: FFmpegVideoEncoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegVideoEncoder.cpp -o $(OBJDIR_DEBUG)/FFmpegVideoEncoder.o

$(OBJDIR_DEBUG)/FFmpegVideoDecoder.o: FFmpegVideoDecoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegVideoDecoder.cpp -o $(OBJDIR_DEBUG)/FFmpegVideoDecoder.o

$(OBJDIR_DEBUG)/FFmpegAudioDecoder.o: FFmpegAudioDecoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegAudioDecoder.cpp -o $(OBJDIR_DEBUG)/FFmpegAudioDecoder.o

$(OBJDIR_DEBUG)/FFmpegOSD.o: FFmpegOSD.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegOSD.cpp -o $(OBJDIR_DEBUG)/FFmpegOSD.o

$(OBJDIR_DEBUG)/FFmpegMuxer.o: FFmpegMuxer.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegMuxer.cpp -o $(OBJDIR_DEBUG)/FFmpegMuxer.o

$(OBJDIR_DEBUG)/FFmpegDemuxer.o: FFmpegDemuxer.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegDemuxer.cpp -o $(OBJDIR_DEBUG)/FFmpegDemuxer.o

$(OBJDIR_DEBUG)/FFmpegAudioResample.o: FFmpegAudioResample.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegAudioResample.cpp -o $(OBJDIR_DEBUG)/FFmpegAudioResample.o

$(OBJDIR_DEBUG)/FFmpegAudioEncoder.o: FFmpegAudioEncoder.cpp
	$(CXX) $(CFLAGS_DEBUG) $(INC_DEBUG) -c FFmpegAudioEncoder.cpp -o $(OBJDIR_DEBUG)/FFmpegAudioEncoder.o

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

$(OBJDIR_RELEASE)/main.o: main.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c main.cpp -o $(OBJDIR_RELEASE)/main.o

$(OBJDIR_RELEASE)/MediaType.o: MediaType.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c MediaType.cpp -o $(OBJDIR_RELEASE)/MediaType.o

$(OBJDIR_RELEASE)/FFmpegVideoScale.o: FFmpegVideoScale.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegVideoScale.cpp -o $(OBJDIR_RELEASE)/FFmpegVideoScale.o

$(OBJDIR_RELEASE)/FFmpegVideoEncoder.o: FFmpegVideoEncoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegVideoEncoder.cpp -o $(OBJDIR_RELEASE)/FFmpegVideoEncoder.o

$(OBJDIR_RELEASE)/FFmpegVideoDecoder.o: FFmpegVideoDecoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegVideoDecoder.cpp -o $(OBJDIR_RELEASE)/FFmpegVideoDecoder.o

$(OBJDIR_RELEASE)/FFmpegAudioDecoder.o: FFmpegAudioDecoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegAudioDecoder.cpp -o $(OBJDIR_RELEASE)/FFmpegAudioDecoder.o

$(OBJDIR_RELEASE)/FFmpegOSD.o: FFmpegOSD.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegOSD.cpp -o $(OBJDIR_RELEASE)/FFmpegOSD.o

$(OBJDIR_RELEASE)/FFmpegMuxer.o: FFmpegMuxer.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegMuxer.cpp -o $(OBJDIR_RELEASE)/FFmpegMuxer.o

$(OBJDIR_RELEASE)/FFmpegDemuxer.o: FFmpegDemuxer.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegDemuxer.cpp -o $(OBJDIR_RELEASE)/FFmpegDemuxer.o

$(OBJDIR_RELEASE)/FFmpegAudioResample.o: FFmpegAudioResample.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegAudioResample.cpp -o $(OBJDIR_RELEASE)/FFmpegAudioResample.o

$(OBJDIR_RELEASE)/FFmpegAudioEncoder.o: FFmpegAudioEncoder.cpp
	$(CXX) $(CFLAGS_RELEASE) $(INC_RELEASE) -c FFmpegAudioEncoder.cpp -o $(OBJDIR_RELEASE)/FFmpegAudioEncoder.o

clean_release: 
	rm -f $(OBJ_RELEASE) $(OUT_RELEASE)
	rm -rf ../bin/Release
	rm -rf $(OBJDIR_RELEASE)

.PHONY: before_debug after_debug clean_debug before_release after_release clean_release

