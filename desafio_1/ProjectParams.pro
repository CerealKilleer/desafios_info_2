QT += core gui
CONFIG += console c++17

SOURCES += \
    src/bitwise_pixel.cpp \
    src/main.cpp

HEADERS += \
    include/bitwise_pixel.hpp \
    include/main.hpp

INCLUDEPATH += include

DESTDIR = bin
OBJECTS_DIR = build
