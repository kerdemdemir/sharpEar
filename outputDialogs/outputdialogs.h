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

#ifndef OUTPUTDIALOGS_H
#define OUTPUTDIALOGS_H

#include <QWidget>
#include "outputDialogs/signaldata.h"
#include "audiperiph/audiperiph.h"
#include <vector>




class QWidget;
class QVBoxLayout;
class QMainWindow;

//** Basic graph related UI elements **//
class QwtSymbol;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlot;

class runningPlot;
class interActionManager;

class outputDialogs : public QWidget
{
    Q_OBJECT
public:
    //outputDialogs();
    outputDialogs(bool isRawData, int sampleSize, QWidget *parent = 0);
    void setSoundData(const std::vector<double>::const_iterator soundData , size_t sizeOfData);
    void drawBasicGraph(std::vector<std::pair<double, double> > & data);


private:

    SignalData signal_data;
    SignalData signal_data_pitch;
    void appendDataToAmplitudeGraph(const std::vector<double>::const_iterator _currentSoundData, size_t sizeOfData);
    void updatePitchGraph();

    interActionManager* hndl_interActionManager;
    //Pitch pitch;
    runningPlot* d_runningPlot;
    runningPlot* d_runningPlotUpper;
    double intervalLength;
    bool   _isRawData;
    int  _sampleSize;

    //** UI Elements **//

    QWidget*     allGraphs;
    QVBoxLayout *outerLayout;
    QMainWindow* pitchWidget;
    QMainWindow* amplitudeWidget;

    //** Basic graph related UI elements **//
    QwtSymbol *symbol;
    QwtPlotCurve *curve;
    QwtPlotGrid *grid;
    QwtPlot* plot;

};

#endif // OUTPUTDIALOGS_H
