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

#ifndef SOUNDDATA_H
#define SOUNDDATA_H

#include <utility/types.h>
#include <utility/commons.h>
#include <iostream>
#include <QPoint>

struct SoundInfo
{
    bool isSound() const
    {
        return soundType != STypes::UNDEFINED;
    }

    bool isSource() const
    {
        return soundType == STypes::SOURCE;
    }

    bool isPulse() const
    {
        return soundType == STypes::PULSE;
    }


    bool isOutput() const
    {
        return boolOutput;
    }

    void setOutput(bool val)
    {
        boolOutput = val;
    }

    int getAngle() const
    {
        return angle;
    }

    void setType(STypes sType)
    {
        soundType = sType;
    }

    double getRadius() const
    {
        return radius;
    }

    STypes getType() const
    {
        return soundType;
    }

    Point getRealPos() const
    {
        return posCm;
    }

    double getDistance() const
    {
        return std::sqrt(posCm.first * posCm.first + posCm.second * posCm.second);
    }

    double getDistance(QPoint pos) const
    {
       return sqrt(pow(getRealPos().first - pos.x(), 2.0) + pow(getRealPos().second - pos.y(), 2.0));
    }

    SoundInfo (Point posGraph, Point posReal, int ang, int rad, STypes sType = STypes::UNDEFINED )
    {
        posScene = posGraph;
        posCm = posReal;
        angle = ang;
        soundType = sType;
        radius = rad;
        boolOutput = false;
    }

    SoundInfo() = default;

    void print( std::string message = std::string() ) const
    {
        if ( message != "" )
        {
            std::cout << "Message: " << message << std::endl;
        }

        std::cout << "SoundInfo <print> Cordinates: " << posCm.first << "," << posCm.second
                  << " Scene Cordinates: " << posScene.first << "," << posScene.second
                  << " Radius: " << radius << " Angle: " << angle << " Type: " << STypes2Str(soundType);

        isOutput() ? (std::cout << " Output Point ") : (std::cout << " Not a output Point");
        std::cout << std::endl;
    }

    bool operator==(const SoundInfo &rhs) const
    {
        if (posCm == rhs.getRealPos())
            return true;
        else
        {
            if (radius == rhs.getRadius() && angle == rhs.getAngle())
                return true;
            else
                return false;
        }
    }

    bool operator<(const SoundInfo &rhs) const
    {
        return (getDistance() < rhs.getDistance());
    }



private:

    Point posScene;
    Point posCm;
    int angle;
    double radius;
    STypes soundType;
    bool boolOutput;


};


namespace std
{
    template<>
    struct hash<SoundInfo>
    {
        size_t operator()(const SoundInfo& v) const
        {
            return make_hash(v.getRealPos());
        }
    };
}

template< typename T >
struct SoundData
{
    using IterType = typename T::iterator;
    using ConstIterType = typename T::const_iterator;
    using value_type = T;
    using iterator = typename T::iterator;

    SoundData (int ID,  Point posGraph, Point posReal, int ang, int rad, STypes sType = STypes::UNDEFINED )
    {
        id = ID;
        info = SoundInfo ( posGraph, posReal, ang, rad, sType );
        dataStatus = SStatus::NOT_INIT;
        dataStart = dataEnd;
    }

    SoundData (int ID, const SoundInfo& newInfo)
    {
        id = ID;
        info = newInfo;
    }

    SoundData() = default;

    bool isPulse() const
    {
        return info.isPulse();
    }

    bool isSound() const
    {
        return info.isSound();
    }

    bool isSource() const
    {
        return info.isSource();
    }

    bool isOutput() const
    {
        return info.isOutput();
    }

    void setOutput( bool input )
    {
        info.setOutput(input);
    }

    Point getRealPos() const
    {
        return info.getRealPos();
    }

    int getAngle() const
    {
        return info.getAngle();
    }

    ConstIterType getData() const
    {
        return dataStart;
    }

    ConstIterType getDataEnd() const
    {
        return dataEnd;
    }

    int getID() const
    {
        return id;
    }

    double getRadius() const
    {
        return info.getRadius();
    }

    SoundInfo& setInfo()
    {
        return info;
    }

    bool isDataExists()
    {
        if ( dataStatus == SStatus::NOT_INIT )
        {
            return false;
        }
        return true;
    }

    STypes getType() const
    {
        return info.getType();
    }

    void setData( IterType begin, IterType end )
    {
        dataStart = begin;
        dataEnd = end;
    }

    void setOutput()
    {
        info.isOutput() = true;
    }

    const SoundInfo& getInfo() const
    {
        return info;
    }

    double calculateSNR( const std::vector<double>& compareData  )
    {
        size_t packetSize = std::distance( dataStart, dataEnd);
        if ( packetSize != compareData.size() )
            std::cout << " SnrManager: Calculate SNR fails sizes are different ";

        double val;
        double noice;
        for ( size_t i = 0; i < packetSize; i++ )
        {
             auto curPower = std::pow ( std::abs(dataStart[i]), 2);
             val += curPower;
             noice += std::pow( dataStart[i].real() - compareData[i], 2 );
        }

        SNRVal = noice/val;
        return SNRVal;
    }

    void print(const std::string& message) const
    {
        std::cout << "Sounddata <print> ID: " << id << " Message: " << message << "SoundInfo: ";
        info.print();
        std::cout << " SNR: " <<  SNRVal << std::endl;
    }

    void setStatus(SStatus status)
    {
        dataStatus = status;
    }

    SStatus getStatus()
    {
        return dataStatus;
    }


    double getDistance(QPoint pos) const
    {
       return info.getDistance(pos);
    }

    bool operator==(const SoundData<IterType> &rhs) const {
        return info == rhs.setInfo();
    }

    bool operator<(const SoundData<IterType> &rhs) const{
        return info.getRealPos() < rhs.getRealPos();
    }

    IterType begin()
    {
        return dataStart;
    }

    IterType end()
    {
        return dataEnd;
    }

private:

    double   SNRVal;
    SStatus  dataStatus;
    IterType dataStart;
    IterType dataEnd;
    int id;
    SoundInfo info;

};



#endif // SOUNDDATA_H
