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

#ifndef PITCHGRAMS_H
#define PITCHGRAMS_H

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <boost/algorithm/string.hpp>
#pragma GCC diagnostic pop

class PitchGrams
{
public:

    PitchGrams(size_t gramCount, size_t stateCount)
    {
        m_gramCount = gramCount;
        m_stateCount = stateCount;
        totalCount = 0;
    }

    void Estimate(const std::vector<size_t>& input)
    {
        if (input.size() <= m_gramCount)
        {
            std::cout << "WARN <pitchGrams> Estimate: input size was less than input " << std::endl;
            return;
        }
        for (size_t i = 0; i < (input.size() - m_gramCount); i++)
        {
            size_t curHash = 0;
            for (size_t k = 0; k < m_gramCount; k++)
            {
                if (input[i] > m_stateCount || input[i + k] > m_stateCount )
                {
                    std::cout << "WARN <pitchGrams> Estimate: data was bigger state range " <<  input[i];
                    std::cout << " " << input[i + k] << std::endl;
                    continue;
                }
                curHash += (pow(m_stateCount+1,k) * input[i + k]);
            }
            ngramCounts[curHash]++;
            totalCount++;
        }
    }

    void TrainingOver()
    {
        for (auto& elem : ngramCounts)
            ngramProbility[elem.first] = (double)elem.second / (double)totalCount;
    }

    double Probability(const std::vector<size_t>& input)
    {
        double totalProbility = 0.0;
        if (input.size() <= m_gramCount)
        {
            std::cout << "WARN <pitchGrams> Probibility: input size was less than input " << std::endl;
            return 0;
        }
        for (size_t i = 0; i < input.size() - m_gramCount; i++)
        {
            size_t curHash = 0;
            for (size_t k = 0; k < m_gramCount; k++)
            {
                curHash += (pow((m_stateCount+1),k) * input[i + k]);
            }
            totalProbility += ngramProbility[curHash];
        }

        return totalProbility;
    }

    double Probability( std::vector<size_t>::const_iterator inputIte,
                        std::vector<size_t>::const_iterator endIte)
    {
        if (inputIte == (endIte - m_gramCount))
            return 0;
        size_t curHash = 0;
        for (size_t k = 0; k < m_gramCount; k++)
        {
            double curVal = *(inputIte + k);
            curHash += (pow((m_stateCount+1),k) * curVal);
        }
        return ngramProbility[curHash];
    }


    void Save(int i)
    {
        std::string outputName = "PGram.txt";
        outputName += std::to_string(i);
        std::fstream outputFile(outputName.c_str(), std::ofstream::out);
        outputFile.precision(std::numeric_limits<double>::digits10);
        for (auto& elem : ngramProbility)
        {
            outputFile << elem.first << "," << elem.second << std::endl;
        }
        outputFile.close();
    }

    int Load(int i)
    {
        std::string inputName = "PGram.txt";
        inputName += std::to_string(i);
        std::ifstream input(inputName.c_str(), std::ifstream::binary);
        input.precision(std::numeric_limits<double>::digits10);
        int counter = 0;
        for (std::string line; std::getline(input, line); )
        {
            std::vector<std::string> strs;
            boost::split(strs, line, boost::is_any_of(","));
            strs[1].pop_back();
            size_t key = std::stoll(strs[0]);
            double value = std::stod(strs[1]);
            ngramProbility[key] = value;
            counter++;
        }
        input.close();
        return counter;
    }

    size_t m_gramCount;
    size_t m_stateCount;
    std::unordered_map < size_t, size_t > ngramCounts;
    std::unordered_map < size_t, double > ngramProbility;
    size_t totalCount;


};

#endif // PITCHGRAMS_H
