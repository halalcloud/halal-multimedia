QT += core
QT -= gui

CONFIG += c++11

TARGET = intelcodecApp
CONFIG += console
CONFIG -= app_bundle

DEFINES += BOOST_LOG_DYN_LINK
INCLUDEPATH +=$$PWD/../inc
INCLUDEPATH +=$$PWD/./include

TEMPLATE = app
DEFINES += LINUX
DEFINES += LINUX32
DEFINES += UNIX __USE_LARGEFILE64 LIBVA_SUPPORT LIBVA_DRM_SUPPORT LIBVA_X11_SUPPOR

SOURCES += \
    pipeline_encode.cpp \
    pipeline_decode.cpp \
    testmain.cpp


HEADERS += \
    include/pipeline_decode.h \
    include/pipeline_encode.h

unix: LIBS += -L/opt/intel/mediasdk/lib/lin_x64/ -ldl -lmfx


INCLUDEPATH += /usr/include/libdrm/

INCLUDEPATH += /opt/intel/mediasdk/include
DEPENDPATH += /opt/intel/mediasdk/include

unix: PRE_TARGETDEPS += /opt/intel/mediasdk/lib/lin_x64/libmfx.a

unix: LIBS += -L$$PWD/ -lsample_common

INCLUDEPATH += $$PWD/sample_common/include
DEPENDPATH += $$PWD/sample_common/include

unix: PRE_TARGETDEPS += $$PWD/libsample_common.a

