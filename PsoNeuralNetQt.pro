#-------------------------------------------------
#
# Project created by QtCreator 2017-08-13T19:11:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PsoNeuralNetQt
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# BOOST
#LIBS += -L"D:\boost_1_61_0\stage\lib" \
#    -lboost_system \
#    -lboost_thread
INCLUDEPATH += "D:\boost_1_61_0"
LIBS += "-LD:/boost_1_61_0/stage/lib/"

# OpenCL
LIBS += -L"D:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v8.0\lib\Win32" \
    -lOpenCL
INCLUDEPATH += "D:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v8.0\include"

INCLUDEPATH += backend frontend

SOURCES += \
    backend/NeuralNet/NeuralNet.cpp \
    backend/PSO/pso.cpp \
    backend/neuralpso.cpp \
    backend/util.cpp \
    frontend/main.cpp \
    frontend/mainwindow.cpp \
    frontend/neuralpsoqtwrapper.cpp

HEADERS += \
    backend/NeuralNet/NeuralNet.h \
    backend/PSO/particles.h \
    backend/PSO/pso.h \
    backend/neuralpso.h \
    backend/util.h \
    frontend/mainwindow.h \
    frontend/neuralpsoqtwrapper.h

FORMS += \
    frontend/mainwindow.ui
