 #-------------------------------------------------
#
# Project created by QtCreator 2014-01-04T16:57:46
#
#-------------------------------------------------

QT       += core gui

QT       += widgets
QT       += script
CONFIG += c++14

QMAKE_CXXFLAGS += -std=gnu++1y -pthread -lpthread

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O0
QMAKE_LFLAGS_RELEASE -= -O1
QMAKE_LFLAGS_RELEASE -= -O0

FORMS    += mainwindow.ui \
    scriptingConsole.ui

SOURCES += main.cpp\
        interactor.cpp\
        mainwindow.cpp\
        environment/enviromentSetup.cpp\
        environment/microphoneNode.cpp\
        environment/roomAtom.cpp\
        environment/roomSimulation.cpp\
        utility/utility.cpp\
        soundIO/IOManager.cpp\
        outputDialogs/outputdialogs.cpp \
        outputDialogs/runningplot.cpp \
        outputDialogs/signaldata.cpp \
    sharpplot.cpp \
    environment/roomDialogs.cpp \
    environment/roomOracle.cpp \
    scriptingConsole.cpp



HEADERS  += mainwindow.h\
            interactor.h\
            environment/enviromentSetup.h\
            environment/microphoneNode.h\
            environment/roomAtom.h\
            environment/roomSimulation.h\
            utility/utility.h\
            outputDialogs/outputdialogs.h \
            outputDialogs/runningplot.h \
            outputDialogs/signaldata.h \
    sharpplot.h \
    utility/workerThread.h \
    environment/roomDialogs.h \
    environment/roomOracle.h \
    utility/soundData.h \
    utility/types.h \
    utility/commons.h \
    soundIO/soundIO.h \
    soundIO/IOManager.h \
    utility/multAccessData.h \
    environment/arrayaparture.h \
    speakerProcess/general.h \
    speakerProcess/statmanager.h \
    speakerProcess/featureExtractor/f0features.h \
    speakerProcess/featureExtractor/featureExtractor.h \
    speakerProcess/featureExtractor/featurelist.h \
    speakerProcess/featureExtractor/MFCCFeatures.h \
    speakerProcess/mlModel/gmmModel.h \
    speakerProcess/mlModel/modelbase.h \
    speakerProcess/mlModel/pitchgrams.h \
    speakerProcess/mlModel/tranierlist.h \
    speakerProcess/featureExtractor/f0highlevelfeatures.h \
    utility/snrmanager.h \
    scriptingConsole.h





TARGET = sharpEar


INCLUDEPATH += "C:\qwt-6.1.2\include"
INCLUDEPATH += "C:\qwt-6.1.2\src"
INCLUDEPATH += "$$_PRO_FILE_PWD_/includes/"
INCLUDEPATH += "C:/range-v3-master/include/"
INCLUDEPATH += "D:/cvOutNoIPP/install/include"
INCLUDEPATH += "D:\soxHeaders"

LIBS += -L"$$_PRO_FILE_PWD_/libs/" -lsoxr -lsndfile-1 -lws2_32 -lqjpeg4 -laubio-4
LIBS += -L"D:\cvOutNoIPP\bin" -lopencv_ml300  -lopencv_highgui300 -lopencv_features2d300 -lopencv_core300

LIBS += -lfftw3-3



CONFIG(debug, debug|release) {
    LIBS += -L"C:\qwt-6.1.2\lib" -lqwtd
}
CONFIG(release, debug|release) {
    LIBS += -L"C:\qwt-6.1.2\lib" -lqwt
}


LIBS += -pthread
CONFIG   += qwt

