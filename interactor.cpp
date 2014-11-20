#include "interactor.h"
#include "mainwindow.h"
#include "environment/roomsimulation.h"
#include "outputDialogs/outputdialogs.h"
#include "environment/enviromentSetup.h"
#include <QDockWidget>
#include <iostream>


void interActionManager::createDialogPanel(QWidget* dialogPanel, QDockWidget* dialogDockWidget)
{
    hndl_enviromentSetup      = new enviromentSetup(dialogDockWidget);
    _sampleSize = SAMPLINGS_FOR_OUTPUTS;
    dialogPanel = hndl_enviromentSetup;
    dialogDockWidget->setWidget(dialogPanel);
}

void
interActionManager::createOutputPanel(QWidget* outputPanel, QDockWidget* dialogDockWidget, bool isRaw)
{
    if (isRaw)
    {
        hndl_raw_outputDialogs      = new outputDialogs(isRaw, SAMPLINGS_FOR_OUTPUTS, dialogDockWidget);
        outputPanel = hndl_raw_outputDialogs;
    }
    else
    {
        hndl_processed_outputDialogs = new outputDialogs(isRaw, SAMPLINGS_FOR_OUTPUTS, dialogDockWidget);
        outputPanel = hndl_processed_outputDialogs;
    }

    dialogDockWidget->setWidget(outputPanel);
}

std::vector<double> *interActionManager::getCurrentCopyOfRawData(bool isPrevBuffer)
{
    if (isPrevBuffer)
    {
        if (_currentIndexOfSoundData == 0)
            return &(_currentCopyOfRawDataPair.second);
        else
            return &(_currentCopyOfRawDataPair.first);
    }
    else
    {
        if (_currentIndexOfSoundData == 1)
            return &(_currentCopyOfRawDataPair.second);
        else
            return &(_currentCopyOfRawDataPair.first);
    }
}


void
interActionManager::takeCopyDataWithSampling(const std::vector< std::complex <double> >::const_iterator soundData,
                                             size_t sizeOfData, bool isRawData )
{
    if ( _sampleSize <= 1 )
        _sampleSize = 1;

    if (isRawData)
    {
        for (unsigned int i = 0; i < sizeOfData; i += _sampleSize)
        {
            _currentCopyOfRawData.push_back( (soundData + i)->real() );
        }
    }
    else
    {
        for (unsigned int i = 0; i < sizeOfData; i += _sampleSize)
        {
            _currentCopyOfProcessedData.push_back( (soundData + i)->real() );
        }
    }
}


void interActionManager::setFocusPoint(double focusDist, std::vector <double> relativeSourceDist)
{
    //hndl_roomSimulation->setFocus(focusDist, relativeSourceDist);
}

void interActionManager::incomingData(const std::vector< std::complex<double> >::const_iterator processedData,
                                      size_t sizeOfPacket, bool isRawData)
{

    takeCopyDataWithSampling( processedData, sizeOfPacket, isRawData ) ;
    if (isRawData)
    {
        _currentCopyOfRawData.clear();
        hndl_raw_outputDialogs->setSoundData( _currentCopyOfRawData.begin(), sizeOfPacket );
    }
    else
    {
        _currentCopyOfProcessedData.clear();
        hndl_processed_outputDialogs->setSoundData( _currentCopyOfProcessedData.begin(), sizeOfPacket );
    }
}


QWidget * interActionManager::createRoom(QRectF boundingRectOfCentralWidget)
{
    hndl_roomSimulation = new roomSimulation(boundingRectOfCentralWidget);
    return hndl_roomSimulation;
}

void interActionManager::reDrawRoom(valuesBasicUserDialog& st_environmentParamaters)
{
    hndl_roomSimulation->reset(st_environmentParamaters);
}

void interActionManager::startVisulution()
{
    hndl_roomSimulation->startVisulution();
}



// Setters and getters

valuesBasicUserDialog *interActionManager::getBasicUserDialogValues() const
{
    return st_shared_environmentParamaters;
}

void interActionManager::setBasicUserDialogValues(valuesBasicUserDialog *value)
{
    st_shared_environmentParamaters = value;
}

std::string interActionManager::getWavFileName() const
{
    return wavFileName;
}

int interActionManager::setWavFileName(std::string value)
{
    //todo Here call to sndlib to open snd file
    wavFileName = std::move(value);
    if (hndl_roomSimulation == NULL)
    {
        std::cout << "Room havent created yet" << std::endl;
        return -1;
    }
    hndl_roomSimulation->setFileName(wavFileName);
    return 0;
}



std::vector<std::pair<int, int> > interActionManager::getCordinatesOfSources() const
{
    return cordinatesOfSources;
}

void interActionManager::setCordinatesOfSources(const std::vector<std::pair<int, int> > &&value)
{
    cordinatesOfSources = std::move(value);
}


