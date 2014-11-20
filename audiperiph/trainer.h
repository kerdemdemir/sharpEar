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

#ifndef TRAINER_H
#define TRAINER_H

#include <map>
#include <audiperiph/pitchgrams.h>
#include <audiperiph/definitions.h>

class Trainer
{
public:

    Trainer()  : gMFCCVec(NUMBER_OF_PEOPLE, GMM<>(6 ,NUMBER_OF_MFCC_FEATURES)),
                 gPitchVec(NUMBER_OF_PEOPLE, GMM<>(6,1)),
                 gResultVec(NUMBER_OF_PEOPLE, GMM<>(3,3)),
                 gPitchGramsVec(NUMBER_OF_PEOPLE, PitchGrams(5, STATE_COUNT))
    {
        methodTruth.resize(NUMBER_OF_PEOPLE);
        resultMFFC.resize(NUMBER_OF_PEOPLE);
        resultPitch.resize(NUMBER_OF_PEOPLE);
        resultGrams.resize(NUMBER_OF_PEOPLE);
        pitchData.resize(NUMBER_OF_PEOPLE);
        filterBankData.resize(NUMBER_OF_PEOPLE);

        outputFile.open("machineLogging.txt", std::ofstream::out);
        correctGuess = 0;
        correctGuessGender = 0;
        totalGuess = 0;
        probibilty = 0;
        probibilityNoice = 0;
    }

    void Estimate(int speakerID, FeatureOutput& features)
    {
        Method methodUse = features.getMethod();
        if (methodUse == Method::ALL_TRAINER || methodUse == Method::PITCH)
            pitchData[speakerID].insert_cols(pitchData[speakerID].n_cols, std::move(features.pitchData));
        if (methodUse == Method::ALL_TRAINER || methodUse == Method::MFCC)
            filterBankData[speakerID].insert_cols(filterBankData[speakerID].n_cols, std::move(features.data));
        if (methodUse == Method::ALL_TRAINER || methodUse == Method::PGRAMS)
            gPitchGramsVec[speakerID].Estimate(features.pitchNormalized);
    }

    void TrainingOver(Method methodUse)
    {
        finalizeTrain(methodUse);
        for (size_t i = 0; i < gPitchGramsVec.size(); i++)
        {
            gPitchGramsVec[i].TrainingOver();
        }
        if (!LOAD)
            Save();
    }

    std::pair<double, double> getResults()
    {
        std::pair<double, double> returnVal = std::make_pair(probibilty, probibilityNoice);
        probibilty = 0;
        probibilityNoice = 0;
        return returnVal;
    }


    void Probability(int speakerID, int noiceID, const FeatureOutput &features);
    void Validate(const std::string& fileName, int state, const FeatureOutput &features);
    void printValidationResults();
    void finalizeValidation(int state, const std::string& fileName);
    void printResults();
    void Save();
    void Load();

    private:

    void finalizeTrain(Method methodUse)
    {
        for (int speakerID = 0; speakerID < NUMBER_OF_PEOPLE; speakerID++)
        {
            if (methodUse == Method::ALL_TRAINER || methodUse == Method::MFCC)
                gMFCCVec[speakerID].Estimate(filterBankData[speakerID], 3);
            if (methodUse == Method::ALL_TRAINER || methodUse == Method::PITCH)
                gPitchVec[speakerID].Estimate(pitchData[speakerID], 3);
        }
    }

    bool
    isMale(std::vector <double>::iterator maxPos)
    {
        size_t distance = std::distance(resultPitch.begin(), maxPos);
        if (distance < 3)
            return false;
        else
            return true;
    }

    std::vector<arma::mat> pitchData;
    std::vector<arma::mat> filterBankData;
    void compareResults(std::vector<double>& resultVec, int state, int noice, Method method);
    std::vector< GMM<> > gMFCCVec;
    std::vector< GMM<> > gPitchVec;
    std::vector< GMM<> > gResultVec;
    std::vector< PitchGrams > gPitchGramsVec;
    std::vector <double> resultMFFC;
    std::vector <double> resultPitch;
    std::vector <double> resultGrams;
    std::vector <double> resultGeneral;
    std::vector < std::map<Method, size_t> >    methodTruth;
    std::map<int, Method>    bestMethod;
    size_t correctGuess;
    size_t correctGuessGender;
    size_t totalGuess;
    double probibilty;
    double probibilityNoice;
    std::map<int, int> trainCounter;
    std::fstream outputFile;
};


#endif // TRAINER_H
