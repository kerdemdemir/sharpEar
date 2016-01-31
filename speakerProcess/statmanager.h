#ifndef STATMANAGER
#define STATMANAGER

#include <range/v3/all.hpp>
#include <unordered_map>
#include <fstream>
#include "mlModel/modelbase.h"
#include "general.h"

struct Counters
{
    size_t correctGuess = 0;
    size_t totalGuess = 0;

    Counters() = default;

    Counters( size_t total )
    {
        totalGuess = total;
        correctGuess = 0;
    }

    Counters( const Counters& rhs )
    {
        totalGuess = rhs.totalGuess;
        correctGuess = rhs.correctGuess;
    }


    void operator++()
    {
        ++correctGuess;
    }

    void operator+=( const Counters& rhs )
    {
        correctGuess += rhs.correctGuess;
        totalGuess += rhs.totalGuess;
    }

    void printResults ( std::fstream& outputFile )
    {
        outputFile << " Total Guess: " << totalGuess << " Correct Guess: " << correctGuess
                   << " Percentage: " << (double)correctGuess/(double)totalGuess * 100.0 << std::endl;

        outputFile << std::string( 40, '*' ) << std::endl;
    }
};



struct ScoreCounter : public Counters
{
    ScoreCounter() : Counters{}, scores{0}
    {

    }

    ScoreCounter( int winner, int guessCount ) : Counters(guessCount), scores{0}, winnerID(winner)
    {

    }

    ScoreCounter( const ScoreCounter& rhs ) : Counters( rhs )
    {
        scores = rhs.scores;
        winnerID = rhs.winnerID;
    }

    void operator+=( const scoreType& rhs )
    {
        auto scoreIndex = sortIndexes<NUMBER_OF_PEOPLE>(rhs);
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            auto score = std::distance ( scoreIndex.begin(), std::find(scoreIndex.begin(), scoreIndex.end(), i ));
            scores[i] += score;
        }

    }

    void operator+=( const ScoreCounter& rhs )
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            scores[i] +=rhs.getScores()[i];
        }
        Counters::operator +=(rhs);
    }


    void checkWinner ()
    {
        auto distance = std::distance ( scores.begin(), std::max_element(scores.begin(), scores.end())) ;
        if ( winnerID == distance)
            ++(*this);
    }


    scoreType getScores() const
    {
        return scores;
    }

private:


    scoreType scores;
    int winnerID = 0;
};

class StatEvaluator
{
public:

    StatEvaluator()
    {
        outputFile.open("SpeakerIdentificationResults.txt", std::ofstream::out);
    }

    ~StatEvaluator()
    {
        outputFile.close();
    }

    void parseResults( const std::vector<std::shared_ptr<ModelBase>>& models)
    {
        for ( auto model : models )
        {
            outputFile << " Model name: " << model->modelName;
            outputFile << " Training time: " << model->miliSecTrainTime << " Estimate Time: "
                       << model->predictionTime << std::endl;
        }
        for ( int i = 200; i <= 1500; i+= 100)
        {
            parseResultsUtterance( models, i);
            modelCounts.clear();
        }

    }

    void parseResultsUtterance( const std::vector<std::shared_ptr<ModelBase>>& models, double utterance  )
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++ )
        {
            //outputFile << " Results for speak ID: " << i << std::endl;
            for ( auto model : models )
            {
                //outputFile << " Model name: " << model->modelName << std::endl;
                auto counter = utteranceStatCalc( model->speakerResultList[i], i, utterance );

                auto modelCounterIter = modelCounts.find(model->modelName);
                if ( modelCounterIter == modelCounts.end() )
                    modelCounts.insert( std::make_pair(model->modelName, counter ) );
                else
                    *(modelCounterIter->second) += *counter;
            }
        }

        outputFile << " Utterance Lenght: " <<  utterance << " Result for all speakers "  << std::endl;
        ScoreCounter allResults;
        for ( auto modelResult : modelCounts)
        {
            outputFile << " Model name: " << modelResult.first << std::endl;
            modelResult.second->printResults(outputFile);
            allResults += *modelResult.second;
        }

        outputFile << " Fusion  Results " << std::endl;
        allResults.printResults(outputFile);


    }

    std::shared_ptr<ScoreCounter> utteranceStatCalc( scoreList& in, int state, double utteranceMilSec )
    {
        int hopLenght = ( sampleRate / hopSize) * ( utteranceMilSec / 1000.0 );

        auto readerChunks = in | ranges::view::chunk(hopLenght);
        auto chunkScore =  std::make_shared<ScoreCounter>(state, readerChunks.size());
        RANGES_FOR( auto chunk, readerChunks )
        {
            RANGES_FOR( auto score, chunk ) // Use range accumulate
                *chunkScore += score;

            chunkScore->checkWinner();
        }

        return chunkScore;
    }

private:

    std::unordered_map< std::string, std::shared_ptr<ScoreCounter> > modelCounts;
    std::fstream outputFile;
};





#endif // STATMANAGER

