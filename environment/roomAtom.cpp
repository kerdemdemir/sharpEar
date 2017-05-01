#include "roomAtom.h"
#include "utility/utility.h"
#include <algorithm>
#include <QBrush>
#include <QPainter>
#include <iostream>
#include <QGraphicsScene>

static constexpr double logCompRatio = 60.0;

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
roomAtom::start2()
{
    m_relativeVal = 0;
    isDrawColor = false;
    sumPart();
    for (auto &elem : m_sumData)
        m_relativeVal += elem.real();
}

int
roomAtom::getAtomDelay( int i, ArrayFocusMode mode )
{
    if ( mode == ArrayFocusMode::NO_FOCUS)
        return m_apartureDist[i];

    return m_apartureDist[i] - m_arrayDelay[i];

}

void
roomAtom::sumPart( )
{
    std::fill(m_sumData.begin(), m_sumData.end(), 0);
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        size_t startingSample = m_sumOffset + getAtomDelay(i, m_array->getMode());
        for (size_t k  = 0; k < m_sumData.size(); k++)
        {
            m_sumData[k] += m_array->getData(i, startingSample + k);
        }
    }
    for (auto& elem : m_sumData)
        elem /= m_array->getElemCount();
}


double
roomAtom::sumSingle()
{
    double result = 0;

    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        size_t startingSample =  m_sumOffset + m_arrayDelay[i];
        if (startingSample < m_array->getData(i).size())
            result += std::abs(m_array->getData(i, startingSample) );

    }

    return result / m_RoomVariables.numberOfMics;
}

void
roomAtom::sumWhole(std::vector< double > & output, ArrayFocusMode mode)
{
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        for (size_t k = 0; k <  output.size(); k++)
        {
            output[k] += m_array->getData(i, k + getAtomDelay(i, mode )).real();
        }
    }

    for (auto& elem : output)
        //elem /= m_array->weightRealSum;
        elem /= m_RoomVariables.numberOfMics;
}

std::vector< double >
roomAtom::sumWhole()
{
    std::vector< double > output(m_SoundParameters.samplePerOutput);
    sumWhole(output);
    return output;
}

void roomAtom::setColor(bool isDraw, double min, double max)
{
    m_relativeVal = ((m_relativeVal - min) / ( max - min ));
    if (ENABLE_LOG_COMPRESSION)
    {
        auto eps = pow(10, (logCompRatio/(-20.0)));

        if (m_relativeVal < eps)
            m_relativeVal = 0;
        else
            m_relativeVal = 20*log(m_relativeVal) + logCompRatio;

        if (m_relativeVal < 0)
            m_relativeVal = 0;
        if (m_relativeVal > logCompRatio)
            m_relativeVal = logCompRatio;
         //_amplitudeSumRatio2RawSound = (((_amplitudeSumRatio2RawSound * 255.0)/60.0));
    }
    isDrawColor = isDraw;
    update();
}


void roomAtom::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    (void)(item);
    (void)(widget);
    QRectF rec = boundingRect();

    if ( isRadiusGuess )
    {
        painter->fillRect( rec, QColor(Qt::yellow) );
        return;
    }
    else if ( m_selfData.getType() == STypes::SOURCE
         || m_selfData.getType() == STypes::SOURCE_NOICE
         || m_selfData.getType() == STypes::PULSE)
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
        if ( isNearField )
        {
            painter->fillRect( rec, QColor(Qt::black) );
            return;
        }
        auto grayScaleVal = m_relativeVal * 255.0;
        if ( ENABLE_LOG_COMPRESSION )
            grayScaleVal /= logCompRatio;

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
