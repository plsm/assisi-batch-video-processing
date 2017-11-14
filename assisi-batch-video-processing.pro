TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += link_pkgconfig
PKGCONFIG = opencv

SOURCES += main.cpp \
    parameters.cpp \
    image.cpp \
    experiment.cpp \
    histogram.cpp

HEADERS += \
    parameters.hpp \
    image.hpp \
    experiment.hpp \
    histogram.hpp
