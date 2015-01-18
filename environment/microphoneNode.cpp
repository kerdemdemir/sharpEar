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
    m_sceneWidth    = 10;

    m_arrayData.resize(m_RoomVariables.numberOfMics);
    for (auto& elem : m_arrayData)
        elem.resize(m_SoundParameters.samplePerOutput ,0);

    m_elemCount = m_RoomVariables.numberOfMics;

    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);

}

void
microphoneNode::elemDistCenter()
{
    double elemDistFromMid = 0;
    m_distCenter.clear();
    m_sceneCenterDist.clear();

    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        if (m_RoomVariables.numberOfMics % 2 == 0)
        {
            elemDistFromMid = m_RoomVariables.numberOfMics / 2;
            m_distCenter.push_back((double(i - elemDistFromMid) + 0.5) * m_RoomVariables.distancesBetweenMics);
            m_sceneCenterDist.push_back((double(i - elemDistFromMid) + 0.5) * m_sceneWidth);
        }
        else
        {
             elemDistFromMid = (m_RoomVariables.numberOfMics / 2);
             m_distCenter.push_back((double(i - elemDistFromMid)) * m_RoomVariables.distancesBetweenMics);
             m_sceneCenterDist.push_back((double(i - elemDistFromMid)) * m_sceneWidth);
        }
    }
}


void
microphoneNode::setElemPos()
{
    m_elemPosCm.clear();
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        double ypos = m_distCenter[i] * sin(m_RoomVariables.angleOfTheMicrophone * GLOBAL_PI / 180);
        double xpos = m_distCenter[i] * cos(m_RoomVariables.angleOfTheMicrophone * GLOBAL_PI / 180);
        ypos += m_center.second;
        xpos += m_center.first;

        m_elemPosCm.push_back(std::make_pair(xpos, ypos));

    }
    print();
}


double
microphoneNode::getDelay(int index, double focusDist, int steeringAngle) const
{
    double dist = focusDist - m_distCenter[index] * sin(steeringAngle * GLOBAL_PI / 180) +  ( pow(m_distCenter[index], 2) / (2.0 * focusDist));
    return dist / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
}

void
microphoneNode::feed(const SoundData<CDataType>& input, const std::vector<double>& weights)
{
    using leapIter =  std::map< int, std::vector < std::vector< std::complex< double > > > >::iterator;

    if (m_arrayData.empty() || m_arrayData.size() != (size_t)m_RoomVariables.numberOfMics)
        std::cout << "microphoneNode: Data is empty or data size different than Array element count" << std::endl;
    bool isFirst = false;

    leapIter leapIte = m_leapData.find(input.getID());
    if (leapIte == m_leapData.end())
    {
        CDataType temp(input.getData(), input.getData() + m_RoomVariables.maximumDelay );
        std::vector<CDataType> tempLeap(m_arrayData.size(), temp);
        m_leapData[input.getID()] = std::move(tempLeap);
        leapIte = m_leapData.find(input.getID());
        isFirst = true;
    }

    std::vector<CDataType> tempLeap = leapIte->second;
    for ( auto &elem : leapIte->second )
        std::fill(elem.begin(), elem.end(), std::complex<double>(0, 0) );
    for (int i = 0; i < m_RoomVariables.numberOfMics; i++)
    {
        double delay = getDelay(i, input.getRadius(), input.getAngle());

        std::cout << "MicrophoneNode <feed> delay : " << delay <<  std::endl;
        size_t delayPos  = isFirst ? (tempLeap[i].size() - delay) : 0;
        CDataConstIter beginIter = input.getData();
        leapIte->second[i].resize(delay);
        for (size_t k = 0; k < (m_arrayData[i].size() + delay); k++)
        {
            if (k + delayPos < tempLeap[i].size())
            {
                m_arrayData[i][k] += tempLeap[i][k] * weights[i];
            }
            else if (k < m_arrayData[i].size())
            {
                SingleCDataType soundData = *beginIter++;
                m_arrayData[i][k] += soundData * weights[i];
            }
            else
            {
                SingleCDataType soundData = *beginIter++;
                leapIte->second[i][k - m_arrayData[i].size()] += soundData * weights[i];
            }
        }
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

