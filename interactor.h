//Copyright (c) 2014,
//Kadir Erdem Demir
//All rights reserved.

//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. All advertising materials mentioning features or use of this software
//   must display the following acknowledgement:
//   This product includes software developed by the <organization>.
//4. Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY Kadir Erdem Demir ''AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef INTERACTOR_H
#define INTERACTOR_H


#include <utility/commons.h>
#include <utility/types.h>
#include <QRectF>

class    enviromentSetup;
class    roomSimulation;
class    outputDialogs;
class    MainWindow;
class    QWidget;
class    QDockWidget;
class    QMainWindow;

//** Will have all the constants which classes uses **//
#define PI_NUMBER 3.141592653589
static const int OFFSET_FROM_TOP = 10;

class interActionManager
{
public:

    void reDrawRoom(valuesBasicUserDialog &st_environmentParamaters);
    void createDialogPanel(QWidget* dialogPanel, QDockWidget* dialogDockWidget);
    void createOutputPanel(QWidget* outputPanel, QDockWidget* dialogDockWidget, bool isRaw);
    QWidget *createRoom(QRectF boundingRect);
    void  startVisulution(); //Triggers when maxElements button pressed
    /****       Data managed by mainUI        ****/

    valuesBasicUserDialog *getBasicUserDialogValues() const;
    void setBasicUserDialogValues(valuesBasicUserDialog *value);

    std::string getWavFileName() const;
    int setWavFileName(std::string value);

    void setFocusPoint(double focusDist, std::vector<double> relativeSourceDist);

    void setCordinatesOfSources(const std::vector<std::pair<int, int> > &&value);
    std::vector<std::pair<int, int> > getCordinatesOfSources() const;

    void incomingData(const std::vector< std::complex<double> >::const_iterator processedData, size_t sizeOfPacket, bool isRawData);

    double getPacketSize();
    double getSampleRate();

    void setTrackingMode( bool trackingMode );
    //multithread related
    std::vector<double>* getCurrentCopyOfRawData(bool isPrevBuffer);
    std::vector<double> _currentCopyOfProcessedData;
    std::vector<double> _currentCopyOfRawData;

    roomSimulation* getRoomSimulation()
    {
        return hndl_roomSimulation;
    }

    //***       Singleton pattern             ****/
    static interActionManager* getDataShareInstance()
    {
        static interActionManager* singletonInstanse = new interActionManager(); // Guaranteed to be destroyed.
        return singletonInstanse;
    }
    void setSoundRef4SNRCalc(const std::vector<double>::const_iterator processedData, size_t sizeOfPacket);
private:

    QMainWindow*        hndl_mainWindow;
    enviromentSetup*    hndl_enviromentSetup;
    roomSimulation*     hndl_roomSimulation;
    outputDialogs*      hndl_raw_outputDialogs;
    outputDialogs*      hndl_processed_outputDialogs;

    valuesBasicUserDialog* st_shared_environmentParamaters;
    /**** Data managed by mainUI        ****/
    std::string wavFileName;
    std::vector<std::pair<int, int> > cordinatesOfSources;
    int    _sampleSize;
    void takeCopyDataWithSampling(const std::vector<std::complex<double> >::const_iterator soundData,
                                                 size_t sizeOfData, bool isRawData );

    std::pair< std::vector<double>, std::vector<double> >  _currentCopyOfRawDataPair;
    int    _currentIndexOfSoundData;

    //** Singleton pattern **//
    interActionManager() {}
    interActionManager(interActionManager const&);              // Don't Implement
    void operator=(interActionManager const&); // Don't implement
};

#endif // DATAMANAGER_H
