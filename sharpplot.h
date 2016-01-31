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

#ifndef SHARPPLOT_H
#define SHARPPLOT_H

#include <vector>
#include <utility/types.h>
#include <qwt_plot.h>
#include <qeventloop.h>

class QWidget;
class QwtSymbol;
class QwtPlotCurve;
class QwtPlotGrid;


class sharpPlot : public QwtPlot
{
     Q_OBJECT

public:
    sharpPlot(std::string plotTitle, std::string curveTitle, bool isLowFilter, QWidget  *parent = 0);
    void drawBasicGraph(CDataType& data, double startPoint = 0.0, double jump = 1.0 );
    void drawBasicGraph(std::vector<std::pair<double, double> > &data);
    void drawBasicGraph( DataType& data, double startPoint = 0.0, double jump = 1.0 );

    void draw(QPolygonF & points, double yAxisMin, double yAxisMax, double xAxisMin, double xAxisMax);
private:

    std::string _plotTitle;
    std::string _curveTitle;
    bool _isLowFilter;

    QwtSymbol *symbol;
    QwtPlotCurve *curve;
    QwtPlotGrid *grid;
    QEventLoop q;

    void lowPassFilter(std::vector<std::pair<double, double> > &data, int windowSize);
};

#endif // SHARPPLOT_H
