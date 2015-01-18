#include "roomAtom.h"
#include "utility/utility.h"
#include <algorithm>
#include <QBrush>
#include <QPainter>
#include <iostream>
#include <QGraphicsScene>


void
roomAtom::start()
{
    m_relativeVal = 0;
    isDrawColor = false;
    sumPart();
    for (auto &elem : m_sumData)
        m_relativeVal += std::abs(elem);
}

void
roomAtom::sumPart( )
{
    const auto& arrayData = m_array.getData();
    std::fill(m_sumData.begin(), m_sumData.end(), 0);


    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        size_t startingSample =  m_sumOffset + m_arrayDelay[i];
        for (size_t k  = 0; k < m_sumData.size(); k++)
        {
                m_sumData[k] += arrayData[i][startingSample + k];
        }
    }

    for (auto& elem : m_sumData)
        elem /= m_array.getElemCount();
}


double
roomAtom::sumSingle()
{
    auto& arrayData = m_array.getData();
    double result = 0;

    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        size_t startingSample =  m_sumOffset + m_arrayDelay[i];
        if (startingSample < arrayData[i].size())
            result += std::abs(arrayData[i][startingSample]);

    }

    return result / m_RoomVariables.numberOfMics;
}

void
roomAtom::sumWhole(std::vector< double > & output)
{
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        for (size_t k = 0; k <  output.size(); k++)
        {
            output[k] += m_array.accessData(i, k, m_arrayDelay[i]).real();
        }
    }

    for (auto& elem : output)
        elem /= m_RoomVariables.numberOfMics;
}

std::vector< double >
roomAtom::sumWhole()
{
    std::vector< double > output(m_SoundParameters.samplePerOutput);
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        for (size_t k = 0; k <  output.size(); k++)
        {
                output[k] += m_array.accessData(i, k, m_arrayDelay[i]).real();
        }
    }

    for (auto& elem : output)
        elem /= m_RoomVariables.numberOfMics;

    return output;
}

void roomAtom::setColor(bool isDraw, double min, double max)
{
    m_relativeVal = ((m_relativeVal - min) / ( max - min ));
    if (ENABLE_LOG_COMPRESSION)
    {
        auto eps = pow(10, (60.0/(-20.0)));

        if (m_relativeVal < eps)
            m_relativeVal = 0;
        else
            m_relativeVal = 20*log(m_relativeVal) + 60;

        if (m_relativeVal < 0)
            m_relativeVal = 0;
        if (m_relativeVal > 60)
            m_relativeVal = 60;
         //_amplitudeSumRatio2RawSound = (((_amplitudeSumRatio2RawSound * 255.0)/60.0));
    }
    isDrawColor = isDraw;
    update();
}


void roomAtom::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    QRectF rec = boundingRect();

    if ( m_selfData.getType() == STypes::SOURCE || m_selfData.getType() == STypes::SOURCE_NOICE )
    {
        painter->setPen(QPen(Qt::red, 10));
        painter->drawPoint(rec.left(), rec.top());
        return;
    }
    else if (m_selfData.getType()  == STypes::MOVING_SOUND)
    {
        painter->setPen(QPen(Qt::green, 10));
        painter->drawPoint(rec.left(), rec.top());
        return;
    }
    else if (m_selfData.getType()  == STypes::NOICE)
    {
        painter->setPen(QPen(Qt::magenta, 10));
        painter->drawPoint(rec.left(), rec.top());
        return;
    }
    else if (m_selfData.isOutput())
    {
        painter->setPen(QPen(Qt::blue, 10));
        painter->drawPoint(rec.left(), rec.top());
        return;
    }

    if (isDrawColor)
    {
        auto grayScaleVal = (((m_relativeVal * 255.0)/60.0));
        QColor colorBlackToWhite = QColor::fromHsv(359, 0, grayScaleVal);
        painter->fillRect(rec, colorBlackToWhite);
        return;

    }
}

//** Graph Related Fuctions**//
QRectF roomAtom::boundingRect() const
{
    if (!ENABLE_RADIAN_ANGLE)
    {
        return QRect(0, 0, m_RoomVariables.pixel4EachAtom,  m_RoomVariables.pixel4EachAtom);
    }
    else
    {
        return QRectF(m_RoomVariables.pixel4EachAtom/-2.0, m_RoomVariables.pixel4EachAtom/-2.0, m_RoomVariables.pixel4EachAtom,  m_RoomVariables.pixel4EachAtom);
    }
}
