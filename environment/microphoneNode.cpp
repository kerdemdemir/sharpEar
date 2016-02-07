#include "microphonenode.h"
#include <algorithm>
#include <iostream>
#include "utility/utility.h"
#include <QBrush>
#include <QPainter>




microphoneNode::microphoneNode(const packetSound &sound, const roomVariables &room)
{
    m_SoundParameters = sound;
    m_RoomVariables   = room;
    m_sceneWidth    = 5;

    for ( int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        m_apartureList.emplace_back(sound, room, i);
    }


    m_elemCount = m_RoomVariables.numberOfMics;

    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);

}

void
microphoneNode::elemDistCenter()
{
    double elemDistFromMid = 0;
    m_sceneCenterDist.clear();

    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        if (m_RoomVariables.numberOfMics % 2 == 0)
        {
            elemDistFromMid = m_RoomVariables.numberOfMics / 2;
            m_apartureList[i].setDistanceFromCenter( (double(i - elemDistFromMid) + 0.5) * m_RoomVariables.distancesBetweenMics );
            m_sceneCenterDist.push_back((double(i - elemDistFromMid) + 0.5) * m_sceneWidth);
        }
        else
        {
             elemDistFromMid = (m_RoomVariables.numberOfMics / 2);
             m_apartureList[i].setDistanceFromCenter( (double(i - elemDistFromMid)) * m_RoomVariables.distancesBetweenMics );
             m_sceneCenterDist.push_back((double(i - elemDistFromMid)) * m_sceneWidth);
        }
    }
}


void
microphoneNode::setElemPos()
{
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        double ypos = m_apartureList[i].getDistCenter() * sin(m_RoomVariables.angleOfTheMicrophone * GLOBAL_PI / 180);
        double xpos = m_apartureList[i].getDistCenter() * cos(m_RoomVariables.angleOfTheMicrophone * GLOBAL_PI / 180);
        ypos += m_center.second;
        xpos += m_center.first;

        m_apartureList[i].setPos( QPoint(xpos, ypos ) );

    }
}


double
microphoneNode::getDelay(int index, double focusDist, int steeringAngle) const
{
    return m_apartureList[index].getDelay(focusDist, steeringAngle, m_mode);
}

double
microphoneNode::getDistDelay(int index, double focusDist ) const
{
    return m_apartureList[index].getDistDelay(focusDist);
}


void
microphoneNode::feed(const SoundData<CDataType>& input, const CDataType& weights)
{
    weightRealSum = 0;
    for ( size_t i = 0; i < m_apartureList.size(); i++ )
    {
        m_apartureList[i].setWeight(weights[i]);
        m_apartureList[i].feed(input);
        weightRealSum += weights[i].real();
    }
}

//** UI related functions **//
//** Graph Related Fuctions**//
QRectF microphoneNode::boundingRect() const
{
    return QRectF (m_sceneWidth * (m_RoomVariables.numberOfMics - 1)/-2, 0, m_sceneWidth * (m_RoomVariables.numberOfMics - 1), 10);
}

void microphoneNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    QRectF rec = boundingRect();
    QBrush blackBrush(Qt::black);
    painter->fillRect(rec, blackBrush);
    painter->setPen(QPen(Qt::red, 5));
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        int yAxis = rec.center().y();
        int xAxis = rec.center().x() + m_sceneCenterDist[i];
        painter->drawPoint( QPoint(xAxis, yAxis) );
    }

}

void microphoneNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void microphoneNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

