#include "outputDialogs/outputdialogs.h"
#include "interactor.h"
#include "outputDialogs/runningplot.h"


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <QMainWindow>
#include <QVBoxLayout>
#include <audiperiph/audiperiph.h>



outputDialogs::outputDialogs(bool isRawData, int sampleSize, QWidget *parent) :
    QWidget(parent), hndl_interActionManager(hndl_interActionManager->getDataShareInstance())
{
    intervalLength = 1;
    _isRawData = isRawData;
    _sampleSize = sampleSize;
    outerLayout = new QVBoxLayout();


    pitchWidget = new QMainWindow();
    amplitudeWidget   = new QMainWindow();

    outerLayout->addWidget(pitchWidget);
    outerLayout->addWidget(amplitudeWidget);

    this->setLayout(outerLayout);
    //pitch.init(44100);
    d_runningPlot = NULL;
    d_runningPlotUpper = NULL;
    this->show();
}


void
outputDialogs::appendDataToAmplitudeGraph(const std::vector<double>::const_iterator _currentSoundData
                                          , size_t sizeOfData)
{
    double timeDiffBetweenSamples = (1.0/44000.0 * _sampleSize);
    double curTime = (signal_data.elapsedDataSize * timeDiffBetweenSamples);

    for (size_t i = 0; i < sizeOfData; i++)
    {
        signal_data.append( QPointF( curTime, *(_currentSoundData + i) ) );
        curTime += timeDiffBetweenSamples;
    }

    if (d_runningPlot == NULL)
    {
        d_runningPlot = new runningPlot(signal_data, -1, 1, QwtPlotCurve::Lines, amplitudeWidget );
        d_runningPlot->setIntervalLength( 10 );
        amplitudeWidget->setCentralWidget(d_runningPlot);
        d_runningPlot->show();
        amplitudeWidget->showMaximized();
    }

    d_runningPlot->updateGraph( signal_data.elapsedDataSize * timeDiffBetweenSamples );
}


void
outputDialogs::updatePitchGraph()
{
    /*std::vector<std::pair<double, double> >& refData = pitch.getF0();

    for (size_t i = 0; i < refData.size(); i++)
    {
        if (refData[i].second != 0)
            signal_data_pitch.append( QPointF( refData[i].first, refData[i].second ) );
    }

    if (d_runningPlotUpper == NULL)
    {
        d_runningPlotUpper = new runningPlot(signal_data_pitch, 0, 500, QwtPlotCurve::Dots, pitchWidget );
        d_runningPlotUpper->setIntervalLength( 10 );
        pitchWidget->setCentralWidget(d_runningPlotUpper);
        d_runningPlotUpper->show();
        pitchWidget->showMaximized();
    }

    d_runningPlotUpper->updateGraph( refData.back().first    );*/
}

void
outputDialogs::setSoundData(const std::vector<double>::const_iterator soundData, size_t sizeOfData)
{
    appendDataToAmplitudeGraph(soundData, sizeOfData);
    //pitch.startProcess(soundData, sizeOfData);
    updatePitchGraph();
}


