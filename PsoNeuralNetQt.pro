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
#win32 {
#    # These lines will link into BOOST.
#    _BOOST_ROOT = $$(BOOST_ROOT)
#    isEmpty(_BOOST_ROOT) {
#        _BOOST_VERSION = $$(BOOST_VERSION)
#        isEmpty(_BOOST_VERSION) {
#            BOOST_LOCATION = C:/boost/boost_1_57_0
#        }
#        else {
#            win32: QWT_LOCATION = C:/boost/boost_$$(BOOST_VERSION)
#        }
#        message(Boost Default: $$BOOST_LOCATION)
#    }
#    else {
#        BOOST_LOCATION = $$(BOOST_ROOT)
#        message(Boost Defined: $$BOOST_LOCATION)
#    }
#    INCLUDEPATH += $$BOOST_LOCATION

#    _LIB_BUILT = 0
#    exists($$BOOST_LOCATION/lib64-msvc-12.0) {
#        message(lib64-msvc-12.0)
#        LIBS += "-L$$BOOST_LOCATION/lib64-msvc-12.0"
#        _LIB_BUILT = 1
#    }

#    exists($$BOOST_LOCATION/stage/lib/boost*.dll) {
#        message(stage exists)
#        LIBS += "-L$$BOOST_LOCATION/stage/lib/"     # Did you compile boost with "stage"?
#        _LIB_BUILT = 1
#    }

#    lessThan(_LIB_BUILT,1) {
#        message(Check your Boost Build)
#    }

#    # Setup path for dlib
#    exists($$(DLIB_PATH)) {
#        message(DLIB Found: $$(DLIB_PATH))
#        INCLUDEPATH += $$(DLIB_PATH)
#    }
#    else {
#        message(DLIB Missing! Check DLIB_PATH)
#    }
#}

# OpenCL
# Load up some CUDA, yo
CUDA_PATH = $$(CUDA_PATH_V8_0)
exists($$CUDA_PATH) {
    message(lOADED CUDA v8.0: $$CUDA_PATH)
} else {
    CUDA_PATH = $$(CUDA_PATH_V7_5)
    exists($$CUDA_PATH) {
        message(lOADED CUDA v7.5: $$CUDA_PATH)
        DEFINES += OPENCL_DEFINED
    } else {
        message(Failed to find CUDA path!  Change the hardcoded path in the .pro)

        # Change CUDA Path to something manual if not loading
        #CUDA_PATH = "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v7.5"
        exists ($$(CUDA_PATH)) {
# fix this too
            #DEFINES += OPENCL_DEFINED
            message(Skipping hardcode: $$(CUDA_PATH))
            CUDA_PATH = $$(CUDA_PATH)
        }
    }
}

INCLUDEPATH += $$CUDA_PATH/include
DEPENDPATH += $$CUDA_PATH/include

win32 {
    !contains(QT_ARCH, i386) {
        exists($$CUDA_PATH/lib/x64) {
            LIBS += -L$$CUDA_PATH/lib/x64 -lOpenCL
        } else {
            message(Failed to load Cuda lib/x64)
            message(Check $$CUDA_PATH for lib/x64 or switch to 32-bit build)
        }
    } else {
        exists($$CUDA_PATH/lib/Win32) {
            LIBS += -L$$CUDA_PATH/lib/Win32 -lOpenCL
        } else {
            message(Failed to load Cuda lib/Win32)
            message(Check $$CUDA_PATH for lib/Win32 or switch to 64-bit build)
        }
    }
}

win32-msvc* {
    message (MSVC in use)
    DEFINES += USE_MSVC

    # Optimization for release
    QMAKE_CXXFLAGS_RELEASE -= -O
    QMAKE_CXXFLAGS_RELEASE -= -O1
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_CXXFLAGS_RELEASE += -Ox

    # Optimization for debugging
    QMAKE_CXXFLAGS_DEBUG += -Od

    # OpenMP
    QMAKE_CXXFLAGS+= /openmp
    QMAKE_LFLAGS +=  /openmp
} else {
    message (None MSVC)
    CONFIG += c++14
    CONFIG += -j # Need to fix this.
    QMAKE_CXXFLAGS_RELEASE += -O3
    QMAKE_CXXFLAGS_DEBUG += -Og

    # OpenMP
    #QMAKE_CXXFLAGS+= -fopenmp
    #QMAKE_LFLAGS +=  -fopenmp
}

macx-g++ {
    message('You re on a mac, dude...')
    DEFINES += USE_MAC
    QMAKE_CXXFLAGS -= -fopenmp
    QMAKE_LFLAGS -= -fopenmp

    QMAKE_CXXFLAGS += -stdlib=libc++
    #QMAKE_CXXFLAGS += -stdlib=libstdc++
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.9
    QMAKE_LFLAGS += -mmacosx-version-min=10.9

    QMAKE_CXXFLAGS_DEBUG -= -Og
    QMAKE_CXXFLAGS_DEBUG += -O0
}

INCLUDEPATH += \
    backend \
    frontend \
    utils \
    Trainer

QWT_LOCATION = $$(QWT_ROOT)
message(Qwt Defined: $$QWT_LOCATION)
include ( $$QWT_LOCATION/features/qwt.prf )

SOURCES += \
    backend/NeuralNet/NeuralNet.cpp \
    backend/PSO/pso.cpp \
    backend/neuralpso.cpp \
    frontend/main.cpp \
    frontend/mainwindow.cpp \
    frontend/neuralnetplot.cpp \
    frontend/innernetnodesinput.cpp \
    backend/teststatistics.cpp \
    utils/custommath.cpp \
    frontend/aboutconfusionmatrixdialog.cpp \
    utils/logger.cpp \
    utils/util.cpp \
    backend/neuralpsostream.cpp \
    utils/statobject.cpp \
    frontend/Trainer/andtrainer.cpp

HEADERS += \
    backend/NeuralNet/NeuralNet.h \
    backend/PSO/particles.h \
    backend/PSO/pso.h \
    backend/neuralpso.h \
    frontend/mainwindow.h \
    frontend/neuralnetplot.h \
    frontend/innernetnodesinput.h \
    backend/teststatistics.h \
    utils/custommath.h \
    frontend/aboutconfusionmatrixdialog.h \
    utils/logger.h \
    utils/util.h \
    backend/neuralpsostream.h \
    utils/statobject.h \
    frontend/Trainer/andtrainer.h

FORMS += \
    frontend/mainwindow.ui \
    frontend/innernetnodesinput.ui \
    frontend/aboutconfusionmatrixdialog.ui
