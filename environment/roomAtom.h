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

    roomAtomRoot(packetSound &sound, roomVariables& room, const microphoneNode* array) : m_array(array)
    {
        m_SoundParameters = sound;
        m_RoomVariables   = room;
        isNearField = false;
        isRadiusGuess = false;
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

    double getDistance( roomAtomRoot* atom ) const
    {
       auto atomPos =  atom->getInfo().getRealPos();
       return getDistance(atomPos, true);
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
        double angle = m_array->getRelativeAngle(realPos);
        double radius = m_array->getRelativeRadius(realPos);
        SoundInfo newInfo(scenePos, realPos, angle, radius);
        m_selfData = newInfo;
        auto nearFieldDistCM = (2 * m_array->getMicLenghtMeter() * m_array->getMicLenghtMeter()) / WAVE_LENGHT_METER * 100;
        if ( getDistance( m_array->getMiddlePos(), true ) < nearFieldDistCM )
            isNearField = true;
        setArrayDelay();
        setApartureDist();
    }

    void setArrayDelay()
    {
        m_arrayDelay.resize(m_array->getElemCount());
        for (int i = 0; i < m_array->getElemCount(); i++)
        {
            m_arrayDelay[i] = m_array->getDelay( i, m_selfData.getRadius(), m_selfData.getAngle());
        }
    }

    void setApartureDist()
    {
        m_apartureDist.resize(m_array->getElemCount());
        for (int i = 0; i < m_array->getElemCount(); i++)
        {            
            m_apartureDist[i] = m_array->getDistDelay(i, getDistance( m_array->getPosition(i), true ));
        }
    }



    SoundInfo& getInfo()
    {
        return m_selfData;
    }

    double getRadius() const
    {
        return m_selfData.getRadius();
    }

    double getAngle() const
    {
        return m_selfData.getAngle();
    }

    void print() const
    {
        m_selfData.print();
    }

    void setType(STypes sType)
    {
        m_selfData.setType(sType);
    }

    void setRoomParams( const roomVariables& newParams )
    {
        m_RoomVariables = newParams;
        auto nearFieldDistCM = (2 * m_array->getMicLenghtMeter() * m_array->getMicLenghtMeter()) / WAVE_LENGHT_METER * 100;
        if ( getDistance( m_array->getMiddlePos(), true ) < nearFieldDistCM )
            isNearField = true;
        setArrayDelay();
        setApartureDist();
    }

    bool isRadiusGuess;
    bool isNearField;

protected:

    SoundInfo m_selfData;
    std::vector<double> m_arrayDelay;
    std::vector<double> m_apartureDist;



    packetSound   m_SoundParameters;
    roomVariables m_RoomVariables;
    const microphoneNode* m_array;

};


///************************************************************************************************//
//**** Real Atom
//**** Deals with real data
///************************************************************************************************//


class roomAtom : public QGraphicsItem, public roomAtomRoot
{

public:

    roomAtom(packetSound &sound, roomVariables &room, const microphoneNode* array)
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
    std::vector<double> sumWhole();

    void start();
    void start2();
    void setColor(bool isDrawColor, double min, double max);

    std::pair< Point , double> getResult() const
    {
        return std::make_pair(m_selfData.getRealPos(), m_relativeVal);
    }

    bool isAtomRadiusCloseBy ( roomAtom* in, int offSet)
    {
        if ( in == nullptr )
            return false;
        if ( std::abs(in->getInfo().getRadius() - getInfo().getRadius()) < offSet )
            return true;
        else
            return false;
    }

    bool isAtomAngleCloseBy ( roomAtom* in, int offSet)
    {
        if ( in == nullptr )
            return false;
        if ( std::abs(in->getInfo().getAngle() - getInfo().getAngle()) < offSet )
            return true;
        else
            return false;
    }

    bool isAtomCloseBy ( roomAtom* in, int offSet)
    {
        if ( in == nullptr )
            return false;
        if ( std::abs(getDistance( getInfo().getRealPos(), true )) < offSet )
            return true;
        else
            return false;
    }

    std::vector< roomAtom* > closeAtomList;
private:

    void sumPart();
    double sumSingle();
    double getAtomDelay( int i, ArrayFocusMode mode );

    std::vector< std::complex<double> >  m_sumData;
    double m_relativeVal;

    bool isDrawColor;
    size_t m_sumOffset;


};




class AtomList
{
public :

    using radAndPair = std::pair <int, std::vector< roomAtom* > >;
    using radAngMap = std::unordered_map < int, std::vector< roomAtom* > >;
    using radAngList = std::vector < radAndPair >;
    using radAngListIte = std::vector < radAndPair >::iterator;
    using radAngMapIte = typename radAngMap::iterator;

    void insert( int rad, int ang, roomAtom* input )
    {
        auto radIte = radiusData.find(rad);
        if (radIte == radiusData.end())
            radiusData[rad].emplace_back(input);
        else
        {
            radIte->second.emplace_back(input);
        }

        auto angIte = angleData.find(ang);
        if (angIte == angleData.end())
            angleData[ang].emplace_back(input);
        else
        {
            angIte->second.emplace_back(input);
        }
    }

    void sort()
    {
        for ( auto& angleListLocal : radiusData )
        {
            std::sort(angleListLocal.second.begin(), angleListLocal.second.end(), []( const roomAtom* lhs,  const roomAtom* rhs){
                return lhs->getAngle() < rhs->getAngle();
            });
            radiusList.push_back(angleListLocal);
        }

        std::sort(radiusList.begin(), radiusList.end(), []( const radAndPair& lhs,  const radAndPair& rhs){
            return lhs.first < rhs.first;
        });

        for ( auto& radiusListLocal : angleData )
        {
            std::sort(radiusListLocal.second.begin(), radiusListLocal.second.end(), []( const roomAtom* lhs,  const roomAtom* rhs){
                return lhs->getRadius() < rhs->getRadius();
            });
            angleList.push_back(radiusListLocal);
        }
        std::sort(angleList.begin(), angleList.end(), []( const radAndPair& lhs,  const radAndPair& rhs){
            return lhs.first < rhs.first;
        });
    }

    std::vector<roomAtom*>& getByAngle( int angle )
    {
        auto ite = angleData.find(angle);
        if (ite == angleData.end())
        {
            auto const itLower = std::lower_bound(angleList.begin(), angleList.end(), angle,
                                                  [](const radAndPair& lhs, int rhs)
            {
                    return lhs.first < rhs;
            });
            if ( itLower != angleList.end() )
                return itLower->second;
            else
                return emptyVec;
        }
        else
        {
            return ite->second;
        }
    }

    roomAtom* getByAngleRadius( int angle, int radius )
    {
        auto angleListTemp = getByAngle(angle);
        auto const itLower = std::lower_bound(angleListTemp.begin(), angleListTemp.end(), radius,
                                              [](const roomAtom* lhs, const int rhs)
        {
            return lhs->getRadius() < rhs;
        });

        if ( itLower != angleListTemp.end() )
            return *itLower;
         return nullptr;
    }

    std::vector<roomAtom*>& getByRadius( int radius )
    {
        auto ite = radiusData.find(radius);
        if (ite == radiusData.end())
        {
            auto const itLower = std::lower_bound(radiusList.begin(), radiusList.end(), radius,
                                                  [](const radAndPair& lhs, const int rhs)
            {
                    return lhs.first < rhs;
            });
            if ( itLower != radiusList.end() )
                return itLower->second;
            else
                return emptyVec;
        }
        else
            return ite->second;
    }



    bool empty()
    {
         return radiusData.empty();
    }

protected:

    std::vector<roomAtom*> emptyVec;
    radAngMap radiusData;
    radAngList radiusList;
    radAngMap angleData;
    radAngList angleList;
};


#endif // SPATIALNODE_H
