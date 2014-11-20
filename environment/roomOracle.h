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

class roomDialogs;

struct snrHelper
{

    snrHelper(const std::pair< double, double >& in)
    {
        snrProbility = in;
    }

    snrHelper() = default;

    snrHelper operator+ (const snrHelper& rhs)
    {
        return snrHelper (std::make_pair(snrProbility.first + rhs.snrProbility.first,
                                          snrProbility.second + rhs.snrProbility.second));
    }

    snrHelper operator/ (int divider)
    {
        return snrHelper (std::make_pair(snrProbility.first / divider,
                                          snrProbility.second / divider));
    }

    double getSecond()
    {
        return snrProbility.second;
    }

    double getFirst()
    {
        return snrProbility.first;
    }

    std::pair< double, double > snrProbility;
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

    void startFeature(const std::map<int, std::vector<double> > &input );
    void feedTrainer(const DataConstIter data, int angle );


    void preprocess(const std::vector<ref_t<SoundData<CDataType> > > &input);
    void postprocess(const std::vector<ref_t<SoundData<CDataType> > > &input);

private:

    void getWeight();
    void parseValidation();

    int m_speakerID;
    int m_noiceID;
    std::map<int, Method> m_bestMethod;
    std::vector< std::pair<int, snrHelper > > m_angleProb;

    std::vector<double> m_weight;
    FeatureOutput m_featureOutput;
    Trainer m_trainer;
    microphoneNode& m_array;

    void feedArray(const std::vector<ref_t<SoundData<CDataType> > > &input, const std::vector<double> &weights);
    void fftWeight();
};


#endif // ROOMORACLE_H
