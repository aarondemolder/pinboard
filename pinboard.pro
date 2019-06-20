TARGET=pinboard

QMAKE_CXXFLAGS += -std=c++14 -pthread

QT+=gui opengl core

SOURCES += src/pinboard.cpp \
            src/arduino-serial-lib.c

HEADERS += include/pinboard.h \
            include/arduino-serial-lib.h

INCLUDEPATH+= include \
                /usr/include/libusb-1.0

LIBS += -lfreenect \
        -lfreenect_sync \
        -lglut \
        -lGL \
        -lGLU \
        -pthread

OBJECTS_DIR=$$PWD/obj

CONFIG += console
