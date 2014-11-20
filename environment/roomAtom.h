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

#ifndef SPATIALNODE_H
#define SPATIALNODE_H

#include <QGraphicsItem>
#include <utility/soundData.h>
#include <unordered_map>
#include <environment/microphonenode.h>
#include <utility/types.h>
#include <iostream>

class QPainter;
class QGraphicsScene;

///************************************************************************************************//
//**** Atom Root Quark Class Start
//**** Just Includes position based calculations
///************************************************************************************************//
class roomAtomRoot
{
public:

    roomAtomRoot(packetSound &sound, roomVariables& room, const microphoneNode& array) : m_array(array)
    {
        m_SoundParameters = sound;
        m_RoomVariables   = room;
    }

    double getDistance(QPointF pos, bool isCm) const
    {
        if (isCm == false)
            pos *= m_RoomVariables.pixel2RealRatio;
        return sqrt(pow(m_selfData.getRealPos().first - pos.x(), 2.0) + pow(m_selfData.getRealPos().second - pos.y(), 2.0));
    }

    double getDistance(Point pos, bool isCm) const
    {
       return getDistance(QPointF(pos.first, pos.second), isCm);
    }

    double getRelativeAngle(Point pos, bool isCm) const
    {
        if (isCm == false)
            pos = Point( pos.first * m_RoomVariables.pixel2RealRatio, pos.second * m_RoomVariables.pixel2RealRatio);
        return atan((pos.first - m_selfData.getRealPos().first) / (pos.second - m_selfData.getRealPos().second)) * 180 / GLOBAL_PI;
    }

    void setNewRoomParams(roomVariables &roomVariables)
    {
        m_RoomVariables = roomVariables;
    }

    void createInfo(const Point& realPos, const Point& scenePos)
    {
        double angle = m_array.getRelativeAngle(realPos);
        double radius = m_array.getRelativeRadius(realPos);
        SoundInfo newInfo(scenePos, realPos, angle, radius);
        m_selfData = newInfo;
        setFocusDelay();
    }

    void setFocusDelay()
    {
        m_arrayDelay.resize(m_array.getElemCount());
        for (int i = 0; i < m_array.getElemCount(); i++)
        {
            m_arrayDelay[i] = m_array.getDelay(i, m_selfData.getRadius(), m_selfData.getAngle());
        }
    }

    SoundInfo& getInfo()
    {
        return m_selfData;
    }

    void print() const
    {
        m_selfData.print();
    }

    void setType(STypes sType)
    {
        m_selfData.setType(sType);
    }

protected:

    void setArrayDelay()
    {
        m_arrayDelay.resize(m_array.getElemCount());
        auto& arrayPos = m_array.getPositions();
        for (size_t i = 0; i < arrayPos.size(); i++)
        {
            double dist = getDistance(arrayPos[i], true);
            m_arrayDelay[i]    =  dist / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
        }
    }

    SoundInfo m_selfData;
    std::vector<double> m_arrayDelay;

    packetSound   m_SoundParameters;
    roomVariables m_RoomVariables;
    const microphoneNode& m_array;

};


///************************************************************************************************//
//**** Real Atom
//**** Deals with real data
///************************************************************************************************//


class roomAtom : public roomAtomRoot, public QGraphicsItem
{
public:

    roomAtom(packetSound &sound, roomVariables &room, const microphoneNode &array)
        : roomAtomRoot(sound, room, array)
    {

        m_sumOffset = m_SoundParameters.samplePerOutput/2 - m_SoundParameters.sumSize/2;
        m_sumData.resize(m_SoundParameters.sumSize, 0);
        isDrawColor = false;
        setAcceptedMouseButtons( Qt::NoButton );
        setActive ( false );

    }

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

    void sumWhole(std::vector<double>& output);
    void sumWhole(std::vector< std::complex<double> >& output);

    void start();
    void setColor(bool isDrawColor, double min, double max);

    std::pair< Point , double> getResult()
    {
        return std::make_pair(m_selfData.getRealPos(), m_relativeVal);
    }

private:

    void sumPart();
    double sumSingle();

    std::vector< std::complex<double> >  m_sumData;
    double m_relativeVal;

    bool isDrawColor;
    size_t m_sumOffset;


};

#endif // SPATIALNODE_H