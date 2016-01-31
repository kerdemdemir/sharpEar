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
#include <environment/microphoneNode.h>
#include <map>
#include <utility/multAccessData.h>
#include <fftw3.h>
#include <speakerProcess/mlModel/tranierlist.h>

class roomDialogs;
class roomSimulation;
class roomAtom;
constexpr double THRESHOLD = 0.4;

using SoundDataRef = ref_t<SoundData<CDataType>>;


class roomOracle
{
public:

    roomOracle(size_t sampleRate, size_t packetSize, int speakerID,
               int noiceID, microphoneNode& array) : m_array(array)
    {
        m_sampleSize = sampleRate;
        m_packetSize = packetSize;
        m_speakerID = speakerID;
        m_noiceID = noiceID;

        std::string trainPath("D:/speakerWavs/train1");
        trainer.initPGrams(0, "F0");
        train(trainer, trainPath);
        trainer4.initPGrams(6, "F6");
        train(trainer4, trainPath);
        soundPosition = nullptr;
        isSoundLocated = false;
    }
    void feedTrainer(const DataConstIter data, int angle );


    void preprocess(const std::vector< SoundDataRef > &input);
    void postprocess(const std::vector< SoundDataRef > &input);
    roomAtom* findSpeakerRadius(const std::vector< roomAtom* >& atomList , TrainerComposer &trainerIn);

    void setRoomSimulation( roomSimulation* mainWindow )
    {
        m_roomSimulation = mainWindow;
    }

private:

    void getWeight();
    void parseValidation();

    int m_speakerID;
    int m_noiceID;
    int m_sampleSize;
    int m_packetSize;
    std::vector< std::pair<int, std::vector<double> > > m_angleProb;

    CDataType m_weight;
    microphoneNode& m_array;
    roomSimulation* m_roomSimulation;
    TrainerComposer trainer;
    TrainerComposer trainer4;

    roomAtom* soundPosition;
    std::vector< double > nullAnglePositions;
    bool isSoundLocated;

    void feedArray(const std::vector< SoundDataRef > &input, const CDataType &weights);
    void fftWeight( );
    void getNoice( );

};


#endif // ROOMORACLE_H
