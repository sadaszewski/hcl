TEMPLATE = lib

CONFIG -= qt

HEADERS += hcl.h hcl_jpeg.h hcl_ffmpeg.h

SOURCES += hcl.cpp hcl_jpeg.cpp hcl_ffmpeg.cpp

LIBS += -ljpeg
