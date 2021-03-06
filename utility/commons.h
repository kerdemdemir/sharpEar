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

#ifndef COMMONS_H
#define COMMONS_H

#include <utility/types.h>

#define GLOBAL_SOUND_SPEED 34300.0
#define GLOBAL_SOUND_SPEED_METER 343.0
#define MAX_FREQUENCY 5000.0
#define MAX_FREQUENCY_KHZ 5.0
#define GLOBAL_PI 3.141592
#define UP_SAMPLE_RATE 8.0

constexpr double  WAVE_LENGHT_METER = GLOBAL_SOUND_SPEED_METER/MAX_FREQUENCY_KHZ;
constexpr int ENABLE_UPSAMPLING = 0  ;
constexpr int ENABLE_LOG_COMPRESSION = 0;
constexpr int ENABLE_RADIAN_ANGLE = 0;
constexpr double EPSILON = 0.00000000001;

const int SAMPLINGS_FOR_OUTPUTS = 1;


enum class STypes
{
    SOURCE = 0,
    NOICE  = 1,
    PULSE = 3,
    MOVING_SOUND = 4,
    SOURCE_NOICE = 5,
    UNDEFINED = 6
};

inline
const char* STypes2Str(STypes type)
{
    if (type == STypes::SOURCE)
        return "Source";
    else if (type == STypes::NOICE)
        return "Noice";
    else if (type == STypes::PULSE)
        return "Pulse";
    else if (type == STypes::MOVING_SOUND)
        return "MovingSound";
    else if (type == STypes::SOURCE_NOICE)
        return "NoicedSound";
    else
        return "Undefined";
}

inline
STypes str2SType( const char* typeStr)
{
    std::string str = typeStr;
    if (str == "Source")
        return STypes::SOURCE;
    else if (str == "Noice")
        return STypes::NOICE;
    else if (str == "Pulse")
        return STypes::PULSE;
    else if (str == "MovingSound")
        return STypes::MOVING_SOUND;
    else if (str == "NoicedSound")
        return STypes::SOURCE_NOICE;
    else
        return STypes::UNDEFINED;
}


enum class SStatus
{
    NOT_INIT = 0,
    JUST_INITED = 0,
    ON_GOING = 1,
    FINISHED = 2
};

struct movingSound
{
    double angle;
    double speed;
};


struct valuesBasicUserDialog
{
    unsigned int micNumber;
    double       distanceBetweenMic;//cm
    unsigned int distance_between_points;//cm
    unsigned int listenRange;//cm
    unsigned int dx_dy;
};




//At least these should be template func
struct roomVariables
{
    int    yPixelCount;
    double pixel2RealRatio;
    int    pixel4EachAtom;
    int    numberOfAtomsIn1D;
    int    maximumDelay;
    int    numberOfMics; //Number of mics
    int    angleDist;
    double distancesBetweenMics;
    int    angleOfTheMicrophone;

};

struct packetSound
{
    double packageDuration;//seconds
    double amplitude;
    double currentOutputTime; //Output of each node probably 1 second or 10ms
    double samplePerOutput;   //bufferSize
    int    sumSize; //Total Window size
    int    samplesPerSec;
};


namespace
{
    template<typename T>
    std::size_t make_hash(const T& v)
    {
        return std::hash<T>()(v);
    }

    void hash_combine(std::size_t& h, const std::size_t& v)
    {
        h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
    }

    template<typename T>
    struct hash_container
    {
        size_t operator()(const T& v) const
        {
            size_t h=0;
            for( const auto& e : v ) {
                hash_combine(h, make_hash(e));
            }
            return h;
        }
    };
}

namespace std
{
    template<typename T, typename U>
    struct hash<pair<T, U>>
    {
        size_t operator()(const pair<T,U>& v) const
        {
            size_t h=make_hash(v.first);
            hash_combine(h, make_hash(v.second));
            return h;
        }
    };

    template<typename... T>
    struct hash<vector<T...>> : hash_container<vector<T...>> {};

    template<typename... T>
    struct hash<map<T...>> : hash_container<map<T...>> {};


}


#endif // COMMONS_H
