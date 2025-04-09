QT += core gui
CONFIG += console c++17

SOURCES += \
    src/bitwise_pixel.cpp \
    src/main.cpp \
    src/process_data.cpp

HEADERS += \
    include/bitwise_pixel.hpp \
    include/constants.hpp \
    include/main.hpp \
    include/process_data.hpp

INCLUDEPATH += include

DESTDIR = bin
OBJECTS_DIR = build
