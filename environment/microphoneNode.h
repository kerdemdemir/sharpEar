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

class QPainter;

class microphoneNode : public QGraphicsItem
{
public:

    microphoneNode(const packetSound &sound, const roomVariables &room);

    void feed( const SoundData<CDataType>& input, const CDataType& weights);

    void
    setElemCount(double micNumber)
    {
        m_RoomVariables.numberOfMics = micNumber;
        m_arrayData.resize(micNumber);
        elemDistCenter();
        setElemPos();
    }

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
        return  atan (   xPos / (yPos ? yPos : EPSILON)  ) * 180 / M_PI ;
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
        for (size_t i = 0; i < m_arrayData.size(); i++)
        {
            std::fill(m_arrayData[i].begin(), m_arrayData[i].end(), 0);
        }
    }

    std::vector<Point>* getMicPositionsInCm()
    {
        return &m_elemPosCm;
    }

    const std::vector<double>& getDistCenter() const
    {
        return m_distCenter;
    }

    const std::vector< Point >& getPositions() const
    {
        return m_elemPosCm;
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
    std::vector<std::vector< std::complex<double> > >&
    getData() const
    {
        return m_arrayData;
    }

    std::complex<double>
    accessData(int aparatureNumber, int index, int delay) const
    {
        if ((index + delay) < (int)m_arrayData[aparatureNumber].size())
        {
            return m_arrayData[aparatureNumber][index + delay];
        }
        else
        {
            std::complex<double> output;
            for (auto& elem : m_leapData)
            {
                if ((index + delay) > (int)m_leapData.size())
                    output += 0;
                else
                    output += elem.second[aparatureNumber][((index + delay) - m_arrayData[aparatureNumber].size())];
            }
            return output;
        }
    }

    int
    getElemCount() const
    {
        return m_elemCount;
    }

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

    void print () const
    {
        std::cout << "Microphone <print> There are " << m_elemCount << " elements in the array"
                  << " Center " << m_center.first << "," << m_center.second << std::endl;
        for (size_t i = 0; i < m_distCenter.size(); i++)
        {
           std::cout << "Microphone <print> Elem Number " << i
                     << " Positions: " << m_elemPosCm[i].first << "," << m_elemPosCm[i].second << std::endl;

        }
    }

    double getMicLenght() const
    {
        return m_elemCount * m_RoomVariables.distancesBetweenMics;
    }

    double getMicLenghtMeter() const
    {
        return m_elemCount * m_RoomVariables.distancesBetweenMics / 100.0;
    }

    double getDelay(int index, double focusDist, int steeringAngle) const;
    CDataType m_weights; //public for now

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


    std::vector< std::vector< std::complex<double> >  >  m_arrayData; // For each microphone I am keeping data
    std::map< int, std::vector < std::vector< std::complex< double > > > > m_leapData; // Leap Data for each source
    std::vector<double>                      m_distCenter; // Distance of each microphone from center
    std::vector<double>                      m_sceneCenterDist; // Distance of each microphone for draw
    std::vector< Point >                     m_elemPosCm;// Spatial position of each microphone
    std::complex<double> fractionalDelayedData(CDataConstIter inData, double delay) const;
};

#endif // MICROPHONENODE_H
