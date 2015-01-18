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

#ifndef ROOMORACLE_H
#define ROOMORACLE_H

#include <complex>
#include <utility/soundData.h>
#include <audiperiph/trainer.h>
#include <environment/microphoneNode.h>
#include <map>
#include <utility/multAccessData.h>

class roomDialogs;

constexpr double THRESHOLD = 0.3;

struct snrHelper
{
    using angleVal = std::pair < int, double >;
    using angleValVec = std::vector < std::pair< int, double > > ;
    using angleValVecIte = std::vector < std::pair< int, double > > ::iterator;

    void insert( const SoundInfo& key, int radius, int angle, const DataType& data)
    {
        SharedDataVec lsharedData = std::make_shared<DataType>(std::move(data));
        if (key.isSource())
        {
            m_source = key;
        }
        else if (key.isSound())
            m_sounds[key] = lsharedData;
        m_soundDigger.insert(radius, angle, lsharedData);
    }

    std::vector<radAngData<SharedDataVec>>* getSourceArc()
    {
       auto radius = m_soundDigger.findClosestRadius(m_source.getRadius());
       auto sourceArc = m_soundDigger.getByRadius(radius);
       return sourceArc;
    }


    void pushResult(int angle, double result)
    {
        resultVec.push_back( std::make_pair(angle, result));
    }

    void startDigging()
    {
        angleValVecIte maxElem = findSource();
        int deltaAngle = 3 * std::abs(resultVec[0].first - resultVec[1].first);
        int maxStart = maxElem->first - deltaAngle;
        int maxEnd = maxElem->first + deltaAngle;
        //std::vector< std::pair < int, double > >
        auto sortedAngleEnergy =  std::move (m_soundDigger.findMax(maxStart, maxEnd));

        for (auto& elem : sortedAngleEnergy)
        {

        }
    }

    angleValVecIte findSource()
    {
        return  std::max_element(resultVec.begin(), resultVec.end(),
            []( angleVal&  lhs,  angleVal& rhs)
            {
                return lhs.second < rhs.second;
            });
    }

    SoundInfo m_source;
    std::unordered_map < SoundInfo,  SharedDataVec > m_sounds;
    radAngDataSummer < std::shared_ptr< std::vector < double > > > m_soundDigger;

    std::vector < std::pair< int, double > > resultVec;
};



class roomOracle
{
public:

    roomOracle(size_t sampleRate, size_t packetSize, int speakerID,
               int noiceID, microphoneNode& array) : m_featureOutput(sampleRate, packetSize), m_array(array)
    {
        m_speakerID = speakerID;
        m_noiceID = noiceID;
        if (LOAD)
        {
            m_trainer.Load();
            parseValidation();
        }
    }

    void startFeature();
    void feedTrainer(const DataConstIter data, int angle );


    void preprocess(const std::vector<ref_t<SoundData<CDataType> > > &input);
    void postprocess(const std::vector<ref_t<SoundData<CDataType> > > &input);

    void insertData(const SoundInfo& key, int radius, int angle, const DataType& data)
    {
        m_digger.insert(key, radius, angle, data);
    }

private:

    void getWeight();
    void parseValidation();

    int m_speakerID;
    int m_noiceID;
    std::map<int, Method> m_bestMethod;
    std::vector< std::pair<int, std::vector<double> > > m_angleProb;

    std::vector<double> m_weight;
    FeatureOutput m_featureOutput;
    Trainer m_trainer;
    microphoneNode& m_array;

    snrHelper m_digger;

    void feedArray(const std::vector<ref_t<SoundData<CDataType> > > &input, const std::vector<double> &weights);
    void fftWeight();
};


#endif // ROOMORACLE_H
