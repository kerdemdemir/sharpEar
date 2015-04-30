#-------------------------------------------------
#
# Project created by QtCreator 2014-01-04T16:57:46
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QMAKE_CXXFLAGS += -std=gnu++0x -pthread -lpthread
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O0

QMAKE_LFLAGS_RELEASE -= -O1
QMAKE_LFLAGS_RELEASE -= -O0

TARGET = sharpEar
TEMPLATE = app


SOURCES += main.cpp\
        interactor.cpp\
        mainwindow.cpp\
        environment/enviromentSetup.cpp\
        environment/microphoneNode.cpp\
        environment/roomAtom.cpp\
        environment/roomSimulation.cpp\
        utility/utility.cpp\
        soundIO/IOManager.cpp\
        audiperiph/audiperiph.cpp\
        outputDialogs/outputdialogs.cpp \
        outputDialogs/runningplot.cpp \
        outputDialogs/signaldata.cpp \
    sharpplot.cpp \
    audiperiph/trainer.cpp \
    environment/roomDialogs.cpp \
    environment/roomOracle.cpp \

HEADERS  += mainwindow.h\
            interactor.h\
            environment/enviromentSetup.h\
            environment/microphoneNode.h\
            environment/roomAtom.h\
            environment/roomSimulation.h\
            utility/utility.h\
            audiperiph/audiperiph.h\
            outputDialogs/outputdialogs.h \
            outputDialogs/runningplot.h \
            outputDialogs/signaldata.h \
    sharpplot.h \
    audiperiph/definitions.h \
    audiperiph/pitchgrams.h \
    audiperiph/trainer.h \
    utility/workerThread.h \
    environment/roomDialogs.h \
    environment/roomOracle.h \
    utility/soundData.h \
    utility/types.h \
    utility/commons.h \
    soundIO/soundIO.h \
    soundIO/IOManager.h \
    utility/multAccessData.h

FORMS    += mainwindow.ui
INCLUDEPATH += "C:\qwt-6.1.2\include"
INCLUDEPATH += "C:\boost_1_55_0"
INCLUDEPATH += "$$_PRO_FILE_PWD_/includes/"





LIBS += -L"$$_PRO_FILE_PWD_/libs/" -lsndfile-1 -lws2_32 -lqjpeg4 -laubio-4 -lmlpack
LIBS += -larmadillo.dll
LIBS += -lfftw3-3 -lblas.dll -llapack.dll -lmsys-xml2-2

CONFIG(debug, debug|release) {
    LIBS += -L"C:\qwt-6.1.2\lib" -lqwtd
    LIBS += -L"C:\boost_1_55_0\stage\lib"
    LIBS += -lboost_program_options-mgw49-mt-d-1_55 -lboost_system-mgw49-mt-d-1_55 -lboost_thread-mgw49-mt-d-1_55 -lboost_filesystem-mgw49-mt-d-1_55
    LIBS += -lboost_random-mgw49-mt-d-1_55 -lboost_math_c99-mgw49-mt-d-1_55
}
CONFIG(release, debug|release) {
    LIBS += -L"C:\qwt-6.1.2\lib" -lqwt
    LIBS += -L"C:\boost_1_55_0\stage\lib" -lboost_system-mgw49-mt-1_55 -lboost_thread-mgw49-mt-1_55
    LIBS += -lboost_program_options-mgw49-mt-1_55 -lboost_filesystem-mgw49-mt-1_55
    LIBS += -lboost_random-mgw49-mt-1_55 -lboost_math_c99-mgw49-mt-1_55
}


LIBS += -pthread
CONFIG   += qwt

