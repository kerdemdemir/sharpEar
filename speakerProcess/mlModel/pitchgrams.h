#ifndef PITCHGRAMS_H
#define PITCHGRAMS_H

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include "modelbase.h"


class PitchGramModel : public ModelBase
{
public:
    PitchGramModel( int gramCount, std::string modelNameParam )
    {
        modelName = modelNameParam;
        m_gramCount = gramCount;
        ngramCounts.resize(NUMBER_OF_PEOPLE);
        ngramProbility.resize(NUMBER_OF_PEOPLE);
    }

    virtual void feed(const std::string& fileName) override
    {
        int state = fileName2State( fileName );
        auto& featureMat = features->getFeatures();
        for ( int i = 0; i < featureMat.rows; i++ )
            peopleF0Data[state].push_back( featureMat.at<double>(i, 0));
    }

    virtual void train () override
    {
        for ( int state = 0; state < NUMBER_OF_PEOPLE;  state++)
        {
            for (size_t i = 0; i < (peopleF0Data[state].size() - m_gramCount); i++)
            {
                size_t curHash = 0;
                for (size_t k = 0; k < m_gramCount; k++)
                {
                    curHash += pow(100,k) * peopleF0Data[state][i + k];
                }
                ngramCounts[state][curHash]++;
            }
        }

        for ( int state = 0; state < NUMBER_OF_PEOPLE; state++)
        {
            auto totalCount = std::accumulate( ngramCounts[state].begin(), ngramCounts[state].end(), 0.0,
                                  [  ](  double sum ,  std::pair<size_t,size_t> rhs )
                                    { return sum + rhs.second;} );
            for ( auto& elem : ngramCounts[state] )
                ngramProbility[state][elem.first] = (double)elem.second / (double)totalCount;
        }
    }


    virtual void predict( int personID ) override
    {
        std::vector< size_t > input;
        auto& featureMat = features->getFeatures();
        for ( int i = 0; i < featureMat.rows; i++ )
            input.push_back( featureMat.at<double>(i, 0) );

        if ( input.empty() )
            return;
        std::array<double,NUMBER_OF_PEOPLE> tempHolder{0};
        if ( input.size() <= m_gramCount )
            return;
        for (size_t i = 0; i < input.size() - m_gramCount; i++)
        {
            size_t curHash = 0;
            for (size_t k = 0; k < m_gramCount; k++)
                curHash += pow(100,k) * input[i + k];

            for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
                tempHolder[i] = ngramProbility[i][curHash];

           // auto scores = sortIndexes<double, NUMBER_OF_PEOPLE>(tempHolder);
            //auto distance = std::distance ( tempHolder.begin(), std::max_element(tempHolder.begin(), tempHolder.end()));
            speakerResultList[personID].push_back(  tempHolder );
        }
    }

    virtual void save() override
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            std::string outputName = "PGram" + modelName + ".txt";
            outputName += std::to_string(i);
            std::fstream outputFile(outputName.c_str(), std::ofstream::out);
            outputFile.precision(std::numeric_limits<double>::digits10);
            for (auto& elem : ngramProbility[i])
            {
                outputFile << elem.first << "," << elem.second << std::endl;
            }
            outputFile.close();
        }
    }

    virtual bool load() override
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            std::string inputName = "PGram" + modelName + ".txt";
            inputName += std::to_string(i);
            std::ifstream input(inputName.c_str(), std::ifstream::binary);
            input.precision(std::numeric_limits<double>::digits10);
            int counter = 0;
            for (std::string line; std::getline(input, line); )
            {
                auto strs = splitString(line);
                strs[1].pop_back();
                size_t key = std::stoi(strs[0]);
                double value = std::stod(strs[1]);
                ngramProbility[i][key] = value;
                counter++;
            }
            input.close();
        }
        return true;
    }


private:

    size_t m_gramCount;
    size_t m_stateCount;
    std::array < std::vector<size_t>, NUMBER_OF_PEOPLE > peopleF0Data;
    std::vector< std::unordered_map < size_t, size_t > > ngramCounts;
    std::vector< std::unordered_map < size_t, double > > ngramProbility;
};


#endif // PITCHGRAMS_H
