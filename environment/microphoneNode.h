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

#ifndef MICROPHONENODE_H
#define MICROPHONENODE_H

#include <vector>
#include <map>
#include <QGraphicsItem>
#include <utility/soundData.h>
#include <environment/arrayaparture.h>
#include <math.h>

class QPainter;

class microphoneNode : public QGraphicsItem
{
public:

    microphoneNode(const packetSound &sound, const roomVariables &room);
    void feed( const SoundData<CDataType>& input, const CDataType& weights);
    void frequencyFeed();

    void
    setDistCount(double distanceBetweenMics, int numberOfElem)
    {
        m_RoomVariables.distancesBetweenMics = distanceBetweenMics;
        m_elemCount = numberOfElem;
        elemDistCenter();
        setElemPos();
    }

    void setNewPos(Point scenePos, Point cmPos)
    {
        m_sceneCenter.first =  scenePos.first;
        m_sceneCenter.second = scenePos.second;

        m_center.first = cmPos.first;
        m_center.second = cmPos.second;

        elemDistCenter();
        setElemPos();
    }

    double
    getRelativeAngle(Point relativePos) const
    {
        double xPos = relativePos.first - m_center.first  ;
        double yPos = relativePos.second - m_center.second ;
        return  atan (   xPos / (yPos ? yPos : EPSILON)  ) * 180.0 / 3.14159265358979323846 ;
    }

    double
    getRelativeRadius(Point relativePos) const
    {
        double xPos = relativePos.first - m_center.first  ;
        double yPos = relativePos.second - m_center.second ;
        return sqrt(pow(xPos, 2) + pow(yPos, 2));
    }


    void renewBuffers()
    {
        for ( auto& aparture : m_apartureList)
        {
            aparture.clearData();
        }
    }

    void
    postFiltering( const CDataType& weights );

    void backupBuffer()
    {
        m_BackUpApartureList = m_apartureList;
        for ( auto& aparture : m_apartureList)
        {
            aparture.clearData();
            aparture.clearLeapData();
        }
    }

    void resetBuffers()
    {
        for ( auto& aparture : m_apartureList)
        {
            aparture.clearData();
            aparture.clearLeapData();
        }
    }

    void takeBackBuffer()
    {
        m_apartureList = m_BackUpApartureList;
    }


    QPoint getPosition( int index ) const
    {
        return m_apartureList[index].getPos();
    }

    Point getMiddlePos() const
    {
        return m_center;
    }

    Point getSceneMiddlePos() const
    {
        return m_sceneCenter;
    }

    const
    ArrayAparture&
    getAparture( int apartureIndex ) const
    {
        return m_apartureList[apartureIndex];
    }

    const
    CDataType&
    getData( int apartureIndex ) const
    {
        return m_apartureList[apartureIndex].getData();
    }

    std::complex<double>
    getData( int apartureIndex, int dataIndex  ) const
    {
        return m_apartureList[apartureIndex].getData( dataIndex );
    }

    double getTotalWeight()
    {
        return std::abs(std::accumulate(m_Allweights.begin(), m_Allweights.end(), std::complex<double>(0)));
    }

    int
    getElemCount() const
    {
        return m_elemCount;
    }

    double
    getDistBetweenElems() const
    {
        return m_RoomVariables.distancesBetweenMics;
    }

    roomVariables getRoomVariables()
    {
        return m_RoomVariables;
    }

    void
    setElemCount( int elemCount )
    {
       m_RoomVariables.numberOfMics = elemCount;
       m_elemCount = elemCount;
    }

    void
    setDistance( int distance )
    {
       m_RoomVariables.distancesBetweenMics = distance;
    }

    void
    multiplyDistance( double constant )
    {
       m_RoomVariables.distancesBetweenMics *= constant;
       setDistCount(m_RoomVariables.distancesBetweenMics, m_elemCount);

    }

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

    double getMicLenght() const
    {
        return m_elemCount * m_RoomVariables.distancesBetweenMics;
    }

    double getMicLenghtMeter() const
    {
        return m_elemCount * m_RoomVariables.distancesBetweenMics / 100.0;
    }

    double totalLength()
    {
        return (m_elemCount - 1) * m_RoomVariables.distancesBetweenMics;
    }

    ArrayFocusMode getMode() const
    {
        return m_mode;
    }

    double getDelay(int index, double focusDist, double steeringAngle) const;
    double getDistDelay(int index, double focusDist) const;
    double weightRealSum;
    double getSteeringDelay(int index, double steeringAngle) const;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:

    void elemDistCenter();
    void setElemPos();

    packetSound   m_SoundParameters;
    roomVariables m_RoomVariables;

    int m_elemCount;
    Point m_center;
    Point m_sceneCenter;
    int m_sceneWidth;
    SoundInfo m_sourceInfo;
    ArrayFocusMode m_mode;


    CDataType m_Allweights;
    std::vector< ArrayAparture > m_apartureList;
    std::vector< ArrayAparture > m_BackUpApartureList;
    std::vector<double>                      m_sceneCenterDist; // Distance of each microphone for draw
    std::complex<double> fractionalDelayedData(CDataConstIter inData, double delay) const;
};

#endif // MICROPHONENODE_H
