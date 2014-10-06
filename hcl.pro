TEMPLATE = lib

CONFIG -= qt

INCLUDEPATH += /opt/local/include

HEADERS += hcl.h hcl_jpeg.h hcl_ffmpeg.h

SOURCES += hcl.cpp hcl_jpeg.cpp hcl_ffmpeg.cpp

LIBS += -L/opt/local/lib

LIBS += -ljpeg
