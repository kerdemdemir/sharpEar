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

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <aubio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <mlpack/methods/gmm/gmm.hpp>
#include <audiperiph/audiperiph.h>
#include <fftw3.h>

using namespace mlpack;
using namespace mlpack::gmm;

#define JUMPSIZE 8
#define STATE_COUNT ((450 - 80) / JUMPSIZE)
#define LOAD 1
#define NUMBER_OF_MFCC_FEATURES 32
#define NUMBER_OF_PEOPLE 6

enum class Method
{
    PITCH = 0,
    MFCC = 1,
    PGRAMS = 2,
    ALL_TRAINER = 3
};


inline
const char* method2Str(Method tempMethod)
{
    if(tempMethod == Method::PITCH)
        return "PITCH";
    else if (tempMethod == Method::MFCC)
        return "MFCC";
    else if (tempMethod == Method::PGRAMS)
        return "PGRAMS";
    else
        return "ALLTRAINER";
}

inline
Method str2Method(std::string elem)
{
    if (elem == "PITCH")
        return Method::PITCH;
    else if (elem == "MFCC")
        return Method::MFCC;
    else if(elem == "PGRAMS")
        return Method::PGRAMS;
    else
        return Method::ALL_TRAINER;
}

struct AubioP
{
    fvec_t *input;
    fvec_t *out;
    uint_t hopSize;
    uint_t windowSize;
    size_t packetSize;
    aubio_pitch_t *pitchPointer;

    AubioP(int sampleRate, size_t packetSizeL)
    {
        char buffer[] = "default";
        wassilence = 1;
        hopSize = 256;
        windowSize = hopSize * 4;
        packetSize = packetSizeL;
        input = new_fvec (hopSize);
        out = new_fvec (1);
        pitchPointer = new_aubio_pitch (buffer, windowSize, hopSize, sampleRate);
    }

    ~AubioP()
    {
        del_aubio_pitch (pitchPointer);
        del_fvec (out);
        del_fvec (input);
        aubio_cleanup ();
    }


    int isSilence()
    {
      if (aubio_silence_detection(input, -40.0)==1)
      {
        if (wassilence==1)
            issilence = 1;
        else
            issilence = 2;
        wassilence=1;
      }
      else
      {
        if (wassilence<=0)
            issilence = 0;
        else
            issilence = -1;
        wassilence=0;
      }

      return issilence;
    }

    void copyData(const std::vector<double>::const_iterator inputIte, size_t curPos)
    {
        for (size_t k = 0; k < hopSize; k++)
        {
            if ((curPos + k) >= packetSize)
                break;

            *(input->data + k) = *(inputIte + k);
        }
    }

    double pitch ()
    {
        aubio_pitch_do (pitchPointer, input, out);
        return (*out->data);
    }

private:
    sint_t wassilence;
    sint_t issilence;

};

struct FFTWParam
{
    double *fftwData;
    fftw_plan fftwPlan;

    FFTWParam(int dataSize)
    {
        fftwData = (double *)fftw_malloc(sizeof (double) * dataSize);
    }

    ~FFTWParam()
    {
        fftw_free(fftwData);
    }

    void dctStep(std::vector<double>& input, arma::mat& output, size_t counter)
    {
        fftwPlan = fftw_plan_r2r_1d(input.size(), input.data(), fftwData, FFTW_REDFT10, FFTW_MEASURE);
        fftw_execute(fftwPlan);

        for (int i = 0; i < NUMBER_OF_MFCC_FEATURES; i++)
        {
           output(i, counter) = fftwData[i];
        }
    }
};


class FeatureOutput
{
public:

    FeatureOutput (int sampleRate, size_t packetSize ) : m_aubioParams( sampleRate, packetSize ),
                                                         m_fftParams( m_aubioParams.hopSize )
    {
        m_gamma.init( 64, sampleRate, m_aubioParams.hopSize );
        m_packetSize = packetSize;
    }


    void calcPitchData()
    {
        m_pitchs.erase(std::remove_if(m_pitchs.begin(), m_pitchs.end(), [](double i) {return !(i >= 80 && i <= 450);}),
                    m_pitchs.end());
        if (feature == Method::PITCH || feature == Method::ALL_TRAINER)
            pitchData.resize(1, m_pitchs.size());
        for (size_t i = 0; i < m_pitchs.size(); i++)
        {
            if (feature == Method::PGRAMS || feature == Method::ALL_TRAINER)
                pitchNormalized.push_back((m_pitchs[i] - 80) / JUMPSIZE);
            if (feature == Method::PITCH || feature == Method::ALL_TRAINER)
                pitchData(0, i) = m_pitchs[i];
        }
    }


    int process( const std::vector<double>::const_iterator rawDataBegin  )
    {
        int counter = 0;
        if (feature == Method::MFCC || feature == Method::ALL_TRAINER)
            data.resize(NUMBER_OF_MFCC_FEATURES, ( m_packetSize / m_aubioParams.hopSize + 1));

        std::vector<double>::const_iterator itePos = rawDataBegin;
        for (size_t i = 0; (i + m_aubioParams.hopSize) < m_packetSize; i += m_aubioParams.hopSize)
        {
            m_aubioParams.copyData(itePos, i);
            if (!m_aubioParams.isSilence())
            {
                if (feature == Method::PITCH || feature == Method::PGRAMS || feature == Method::ALL_TRAINER)
                    m_pitchs.push_back(m_aubioParams.pitch());

                if (feature == Method::MFCC || feature == Method::ALL_TRAINER)
                {
                    auto& gammaCoef = m_gamma.startProcess(itePos);
                    m_fftParams.dctStep(gammaCoef, data, counter);
                }

                ++counter;
            }
            itePos =  itePos + m_aubioParams.hopSize;
        }

        if (feature == Method::MFCC || feature == Method::ALL_TRAINER)
            data.resize(NUMBER_OF_MFCC_FEATURES, counter);

        if (feature == Method::PITCH || feature == Method::PGRAMS || feature == Method::ALL_TRAINER)
            calcPitchData();
        return 0;
    }

    void clear()
    {
        m_pitchs.clear();
        pitchData.clear();
        data.clear();
        pitchNormalized.clear();
    }

    std::vector<size_t> pitchNormalized;
    arma::mat pitchData;
    arma::mat data;

    Method getMethod() const
    {
        return feature;
    }

    void setMethod(const Method &value)
    {
        feature = value;
    }

private:

    Method feature;
    std::vector<double> m_pitchs;
    size_t m_packetSize;
    Gammatone m_gamma;
    AubioP m_aubioParams;
    FFTWParam m_fftParams;

};



#endif // DEFINITIONS_H

