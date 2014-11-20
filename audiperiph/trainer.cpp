#include <audiperiph/trainer.h>
#include <algorithm>

void Trainer::printResults()
{
    outputFile << "*************************" << std::endl;
    outputFile << "Final Resultsss  areeeee :" << std::endl;
    outputFile << "*************************" << std::endl;
    outputFile << "Correct Guess Gender" << correctGuessGender << " Total Guess " << totalGuess << std::endl;
    outputFile << "Percentage Gender" << (double)correctGuessGender / (double)totalGuess << std::endl;
    outputFile << "Correct Guess Speaker" << correctGuess << " Total Guess " << totalGuess << std::endl;
    outputFile << "Percentage Speaker" << (double)correctGuess / (double)totalGuess << std::endl;
    outputFile << "*************************" << std::endl;
    outputFile.close();
}

void Trainer::Validate(const std::string& fileName, int state, const FeatureOutput &features)
{
    std::fill(resultMFFC.begin(), resultMFFC.end(), 0);
    std::fill(resultPitch.begin(), resultPitch.end(), 0);
    std::fill(resultGrams.begin(), resultGrams.end(), 0);


    for (size_t i = 0; i < features.data.n_cols; i++)
    {
        for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
        {
            double tempProbility = gMFCCVec[k].Probability(features.data.col(i));
            resultMFFC[k] += tempProbility;
        }
    }

    for (size_t i = 0; i < features.pitchData.n_cols; i++)
    {
        for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
        {
            double tempProbility = gPitchVec[k].Probability(features.pitchData.col(i));
            resultPitch[k] += tempProbility;
        }
    }

    for (size_t i = 0; i < features.pitchNormalized.size(); i++)
    {
        for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
        {
            double tempProbility = gPitchGramsVec[k].Probability(features.pitchNormalized.begin() + i,
                                                                 features.pitchNormalized.end());
            resultGrams[k] += tempProbility;
        }
    }

    finalizeValidation(state, fileName);
}

void Trainer::finalizeValidation(int state, const std::string& fileName)
 {
     double mfccSum =std::accumulate(resultMFFC.begin(),resultMFFC.end(),0.0);
     double pitchSum =std::accumulate(resultPitch.begin(),resultPitch.end(),0.0);
     double pGramsSum =std::accumulate(resultGrams.begin(),resultGrams.end(),0.0);

     for (int k = 0; k < NUMBER_OF_PEOPLE; k++)
     {
         resultMFFC[k] = resultMFFC[k] / mfccSum;
         resultPitch[k] = resultPitch[k] / pitchSum;
         resultGrams[k] = resultGrams[k] / pGramsSum;
     }
     auto maxPitch = std::max_element(resultPitch.begin(), resultPitch.end());
     bool isFemale = !isMale(maxPitch);


     auto maxMFCC = isFemale ? std::max_element(resultMFFC.begin(), resultMFFC.end()) :
                        std::max_element(resultMFFC.begin()+3, resultMFFC.end());

     auto maxGrams = isFemale ? std::max_element(resultGrams.begin(), resultGrams.end()) :
                              std::max_element(resultGrams.begin()+3, resultGrams.end());


     if (std::distance(resultMFFC.begin(), maxMFCC) == state)
     {
         methodTruth[state][Method::MFCC]++;
     }
     if (std::distance(resultPitch.begin(), maxPitch) == state)
     {
         methodTruth[state][Method::PITCH]++;
     }
     if (std::distance(resultGrams.begin(), maxGrams) == state)
     {
         methodTruth[state][Method::PGRAMS]++;
     }



     outputFile << "*********************************************" << std::endl;
     outputFile <<  "Validation real excepted state: " << state << "fileName " << fileName << std::endl;
     outputFile <<  "Validation MFCC Result was: " << std::distance(resultMFFC.begin(), maxMFCC) << std::endl;
     outputFile <<  "Validation Pitch Result was: " << std::distance(resultPitch.begin(), maxPitch) << std::endl;
     outputFile <<  "Validation PGRAM Result was: " << std::distance(resultGrams.begin(), maxGrams) << std::endl;


     outputFile << "*** All results ***" << std::endl;
     for (auto elem : resultMFFC)
         outputFile << elem << std::endl;

     outputFile << "*** Pitch ***" << std::endl;
     for (auto elem : resultPitch)
         outputFile << elem << std::endl;

     outputFile << "*** PGrams ***" << std::endl;
     for (auto elem : resultGrams)
         outputFile << elem << std::endl;



     outputFile << "*********************************************" << std::endl;
     outputFile.flush();
 }

void Trainer::printValidationResults()
{
    std::cout << "Validation is over " << std::endl;
    outputFile << "Validation is over " << std::endl;
    std::ofstream methodOutput("speakerMethod.txt");
    for (size_t i = 0; i < methodTruth.size(); i++)
    {
        size_t bestCount = methodTruth[i][Method::MFCC];
        Method tempBestMethod = Method::MFCC;
        outputFile << "For person number " << (i+1) << std::endl;
        outputFile << "MFCC was correct for " <<methodTruth[i][Method::MFCC] << " times" << std::endl;
        if (methodTruth[i][Method::PITCH] > bestCount)
            tempBestMethod = Method::PITCH;
        outputFile << "PITCH was correct for " << methodTruth[i][Method::PITCH] << " times" << std::endl;
        if (methodTruth[i][Method::PGRAMS] > bestCount)
            tempBestMethod = Method::PGRAMS;
        outputFile << "PGRAMS was correct for " << methodTruth[i][Method::PGRAMS] << " times" << std::endl;
        methodOutput << i << "," << (int)tempBestMethod << std::endl;
    }

    methodOutput.close();
    std::cout << "**********************************" << std::endl;
}

void Trainer::Probability(int speakerID, int noiceID, const FeatureOutput &features)
{
    std::vector < double > resultVec(NUMBER_OF_PEOPLE);
    Method speakerMethod = features.getMethod();
    if (speakerMethod == Method::MFCC)
    {
        for (size_t i = 0; i < features.data.n_cols; i++)
        {
            for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
            {
                double tempProbility = gMFCCVec[k].Probability(features.data.col(i));
                resultVec[k] += tempProbility;
            }
        }
    }
    else if (speakerMethod == Method::PITCH)
    {
        for (size_t i = 0; i < features.pitchData.n_cols; i++)
        {
            for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
            {
                double tempProbility = gPitchVec[k].Probability(features.pitchData.col(i));
                resultVec[k] += tempProbility;
            }
        }
    }
    else
    {
        for (size_t i = 0; i < features.pitchNormalized.size(); i++)
        {
            for (size_t k = 0; k < NUMBER_OF_PEOPLE; k++)
            {
                double tempProbility = gPitchGramsVec[k].Probability(features.pitchNormalized.begin() + i,
                                                                     features.pitchNormalized.end());
                resultVec[k] += tempProbility;
            }
        }
    }
    compareResults(resultVec, speakerID, noiceID, speakerMethod);
}






void
Trainer::compareResults(std::vector<double>& resultVec, int state, int noice, Method method)
{
    double resultSum =std::accumulate(resultVec.begin(),resultVec.end(),0.0);

    for (int k = 0; k < NUMBER_OF_PEOPLE; k++)
        resultVec[k] = resultVec[k] / resultSum;
    auto maxResult = std::max_element(resultVec.begin(), resultVec.end());
    probibilty = resultVec[state];
    probibilityNoice = resultVec[noice];
    size_t distance = std::distance(resultVec.begin(), maxResult);
    bool correctTag = false;
    if ((int)distance == state)
    {
        correctTag = true;
        correctGuess++;
    }

    if (state >= 3 &&  distance >= 3)
        correctGuessGender++;
    else if (state < 3 && distance < 3)
        correctGuessGender++;
    totalGuess++;

    if (!correctTag)
    {
        outputFile << "*** Wrong guess " << state <<  " expected ***" << std::endl;
        outputFile << "*** Method was " << (int)method << " ***" << std::endl;
        for (auto elem : resultVec)
            outputFile << elem << std::endl;
        outputFile << "*********************************************" << std::endl;
    }

}


/***********************************************************************************/
/***********************************************************************************/
/***********************************************************************************/
void Trainer::Save()
{
    std::string mffcMeans = "mffcMean.txt";
    std::string pitchMeans = "pitchMean.txt";
    std::string mffcCovariance = "mffcCovariance.txt";
    std::string pitchCovariance = "pitchCovariance.txt";
    std::string mfccWeights = "mfccWeights.txt";
    std::string pitchWeights = "pitchWeights.txt";

    for (int k = 0; k < NUMBER_OF_PEOPLE; k++)
    {
        gPitchGramsVec[k].Save(k);
        std::string index = std::to_string(k);
        std::string mffcMeansName = mffcMeans + index;
        std::string pitchMeansName = pitchMeans + index;
        std::string mffcCovarianceName =  mffcCovariance + index;
        std::string pitchCovarianceName = pitchCovariance + index;
        std::string mfccWeightsName =  mfccWeights + index;
        std::string pitchWeightsName = pitchWeights + index;


        for (size_t i = 0; i < gMFCCVec[k].Means().size(); i++)
        {
            std::string indexInner = std::to_string(i);
            std::string mffcMeansInnerName = mffcMeansName + "_";
            mffcMeansInnerName += indexInner;
            arma::Col<double>& tempArma = gMFCCVec[k].Means()[i];
            tempArma.save(mffcMeansInnerName);
        }

        for (size_t i = 0; i < gPitchVec[k].Means().size(); i++)
        {
            std::string indexInner = std::to_string(i);
            std::string pitchMeansInnerName = pitchMeansName + "_";
            pitchMeansInnerName += indexInner;
            arma::Col<double>& tempArma = gPitchVec[k].Means()[i];
            tempArma.save(pitchMeansInnerName);
        }


        for (size_t i = 0; i < gMFCCVec[k].Covariances().size(); i++)
        {
            std::string indexInner = std::to_string(i);
            std::string mffcCovarianceInnerName = mffcCovarianceName + "_";
            mffcCovarianceInnerName += indexInner;
            arma::mat& tempArma = gMFCCVec[k].Covariances()[i];
            tempArma.save(mffcCovarianceInnerName);
        }

        for (size_t i = 0; i < gPitchVec[k].Covariances().size(); i++)
        {
            std::string indexInner = std::to_string(i);
            std::string pitchCovarianceInnerName = pitchCovarianceName + "_";
            pitchCovarianceInnerName += indexInner;
            arma::mat& tempArma = gPitchVec[k].Covariances()[i];
            tempArma.save(pitchCovarianceInnerName);
        }

        arma::Col<double>& tempArmaMFCCWeight = gMFCCVec[k].Weights();
        tempArmaMFCCWeight.save(mfccWeightsName);


        arma::Col<double>& tempArmaPitchWeight = gMFCCVec[k].Weights();
        tempArmaPitchWeight.save(pitchWeightsName);
    }
}

void Trainer::Load()
{
    std::string mffcMeans = "mffcMean.txt";
    std::string pitchMeans = "pitchMean.txt";
    std::string mffcCovariance = "mffcCovariance.txt";
    std::string pitchCovariance = "pitchCovariance.txt";
    std::string mfccWeights = "mfccWeights.txt";
    std::string pitchWeights = "pitchWeights.txt";

    for (int k = 0; k < NUMBER_OF_PEOPLE; k++)
    {
        gPitchGramsVec[k].Load(k);

        std::string index = std::to_string(k);
        std::string mffcMeansName = mffcMeans + index;
        std::string pitchMeansName = pitchMeans + index;
        std::string mffcCovarianceName =  mffcCovariance + index;
        std::string pitchCovarianceName = pitchCovariance + index;
        std::string mfccWeightsName =  mfccWeights + index;
        std::string pitchWeightsName = pitchWeights + index;

        size_t i = 0;
        while(true)
        {
            std::string indexInner = std::to_string(i);
            std::string mffcMeansInnerName = mffcMeansName + "_";
            mffcMeansInnerName += indexInner;
            arma::Col<double> tempArma;
            if (!tempArma.load(mffcMeansInnerName))
                break;
            gMFCCVec[k].Means()[i] = std::move(tempArma);
            i++;
        }

        i = 0;
        while(true)
        {
            std::string indexInner = std::to_string(i);
            std::string pitchMeansInnerName = pitchMeansName + "_";
            pitchMeansInnerName += indexInner;
            arma::Col<double> tempArma;
            if (!tempArma.load(pitchMeansInnerName))
                break;
            gPitchVec[k].Means()[i] = std::move(tempArma);
            i++;
        }

        i = 0;
        while(true)
        {
            std::string indexInner = std::to_string(i);
            std::string mffcCovarianceInnerName = mffcCovarianceName + "_";
            mffcCovarianceInnerName += indexInner;
            arma::mat tempArma;
            if (!tempArma.load(mffcCovarianceInnerName))
                break;
            gMFCCVec[k].Covariances()[i] = std::move(tempArma);
            i++;
        }

        i = 0;
        while(true)
        {
            std::string indexInner = std::to_string(i);
            std::string pitchCovarianceInnerName = pitchCovarianceName + "_";
            pitchCovarianceInnerName += indexInner;
            arma::mat tempArma;
            if(!tempArma.load(pitchCovarianceInnerName))
                break;
            gPitchVec[k].Covariances()[i] = std::move(tempArma);
            i++;
        }

        arma::Col<double> tempArmaMFCCWeight;
        tempArmaMFCCWeight.load(mfccWeightsName);
        gMFCCVec[k].Weights() = std::move(tempArmaMFCCWeight);

        arma::Col<double> tempArmaPitchWeight;
        tempArmaMFCCWeight.load(pitchWeightsName);
        gPitchVec[k].Weights() = std::move(tempArmaPitchWeight);
    }
}
