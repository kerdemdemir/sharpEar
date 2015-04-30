#include "sharpplot.h"
#include <QTimer>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_symbol.h>
#include <qwt_legend.h>
#include <QWidget>
#include <qwt_plot_renderer.h>

sharpPlot::sharpPlot(std::string plotTitle, std::string curveTitle, bool isLowFilter,
                     QWidget *parent) : QwtPlot(QwtText(QString(plotTitle.c_str())), parent)
{
    _plotTitle = plotTitle;
    _curveTitle = curveTitle;
    _isLowFilter = isLowFilter;
    this->resize( 600, 400 );
    this->show();
}

void
sharpPlot::lowPassFilter(std::vector<std::pair <double, double> >& data, int windowSize)
{
    double curSum = 0;
    std::vector<double> outputVec (windowSize/2 + data.size());

    for (int i = 0; i <= windowSize; i++)
    {
        if (i >= (windowSize+1)/2)
            outputVec[i - (windowSize+1)/2] = curSum/windowSize;
        if (i == windowSize)
            break;
        curSum += data[i].second ;

    }

    for (size_t i = (windowSize+1)/2; i < outputVec.size(); i++)
    {
        if ( data.size() > (i + windowSize/2) )
            curSum += (data[i + windowSize/2].second - data[i - (windowSize+1)/2].second);
        else
            curSum -= data[i - (windowSize+1)/2].second;
        outputVec[i] = curSum/windowSize;
    }

    for (size_t i = 0; i < data.size(); i++)
    {
        data[i].second = outputVec[windowSize/4 + i];
    }
}


void sharpPlot::drawBasicGraph(CDataType& data, double startPoint, double jump )
{
    QPolygonF points;
    for (size_t i = 0; i < data.size(); i++)
    {
        points << QPointF( startPoint + i * jump, std::abs(data[i]) );
    }

    auto minMax = std::minmax_element(data.begin(), data.end(), [](SingleCDataType a, SingleCDataType b)
    {
         return std::abs(a) < std::abs(b);
    });

    draw( points, std::abs(*minMax.first),  std::abs(*minMax.second), startPoint, startPoint + jump * data.size() );
}


void
sharpPlot::drawBasicGraph(std::vector<std::pair <double, double> >& data)
{
    QPolygonF points;

    if (_isLowFilter)
        lowPassFilter(data, 10);

    for (auto elem : data)
    {
        points << QPointF( elem.first, elem.second );
    }

    auto minMax = std::minmax_element(data.begin(), data.end(), [](std::pair <double, double> a, std::pair <double, double> b)
    {
         return a.second < b.second;
    });

    draw( points, minMax.first->second, minMax.second->second, data.front().first, data.back().first );
}


void
sharpPlot::draw(QPolygonF & points, double yAxisMin, double yAxisMax, double xAxisMin, double xAxisMax)
{
    QTimer tT;
    QwtPlotRenderer renderer;
    tT.setSingleShot(true);
    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));

    setTitle( _plotTitle.c_str() );
    setCanvasBackground( Qt::white );
    setAxisScale( QwtPlot::yLeft, yAxisMin,  yAxisMax);
    setAxisScale( QwtPlot::xBottom, xAxisMin, xAxisMax );
    insertLegend( new QwtLegend() );

    grid = new QwtPlotGrid();
    grid->attach( this );

    curve = new QwtPlotCurve();
    curve->setTitle( _curveTitle.c_str() );
    curve->setPen( Qt::black, 1 ),
    curve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    curve->setSamples( points  );
    curve->attach( this );

    updateAxes();
    update();
    replot();
    QString curTitle = _curveTitle.c_str(); curTitle += ".pdf";
    renderer.renderDocument(this, curTitle, QSizeF(100, 100));
    repaint();
    tT.start(1000);
    q.exec();
    if(tT.isActive())
    {
       tT.stop();
    }
}


