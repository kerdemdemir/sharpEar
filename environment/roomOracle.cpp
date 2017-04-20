#include <environment/roomOracle.h>
#include <sharpplot.h>
#include <speakerProcess/mlModel/tranierlist.h>
#include "environment/roomSimulation.h"
#include "environment/roomAtom.h"
#include "utility/utility.h"
#include "utility/sortedbestpicklist.h"
constexpr int IS_DRAW = 0;
constexpr int IS_SIMPLE_POWER = 0;
constexpr int IS_F0_POWER = 1;
constexpr int THRESHOLD_POWER = 0.0002;

roomOracle::roomOracle(size_t sampleRate, size_t packetSize, int speakerID, int noiceID, microphoneNode &array): scorer(2000, 25)
{
    m_array = &array;
    m_sampleSize = sampleRate;
    m_packetSize = packetSize;
    m_speakerID = speakerID;
    m_noiceID = noiceID;

    std::string trainPath("D:/speakerWavs/train1");
    trainer.initP0Grams();
    //trainer.initMFCC();
    train(trainer, trainPath);

    trainerRadius.initP0Power(5);
//    trainerRadius.initPGrams(2, "F2");
//    //trainerRadius.initHighLevelGMM();
//    train(trainerRadius, trainPath);
    soundPosition = nullptr;
    isSoundLocated = false;
    isRadiusLocated = false;
    isAngleLocated = false;
    isManualMode = true;
    m_lookAngle = 0;
    angle = -999;
    radius = -999;
    m_resultFile.open("LocationingResultFile.txt", std::fstream::out);
}

void roomOracle::preprocess(const std::vector< SoundDataRef > &input, int packetCount )
{
     if ( isSoundLocated )
    {
        feedArray(input, m_weight);
        return;
    }
    m_weight.resize(m_array->getElemCount(), 1);

    if ( isManualMode )
    {
        if ( !nullAnglePositions.empty() )
            fftWeight();
        auto soundPos = feedArray(input, m_weight);
        isSoundLocated = true;
        SoundData<CDataType>& dataOriginal = soundPos;
        auto atomInRadius = m_roomSimulation->findAtomPolarImpl(dataOriginal.getInfo().getRadius(), dataOriginal.getInfo().getAngle());
        auto snrVal = dataOriginal.calculateSNR(atomInRadius->sumWhole());
        dataOriginal.getInfo().print( QString(" SNR val : %1 ").arg(snrVal).toStdString());
        return;
    }

    roomAtom* soundPositionLocal = nullptr;
    auto originalSound = feedArray(input, m_weight);
    if ( packetCount == 0  )
         return;
    std::cout << " Is Radius Located " << isRadiusLocated << "Is Angle Located " << isAngleLocated << std::endl;
    if ( !isRadiusLocated )
    {
        double radiusAngle = angle;
        if ( !isAngleLocated )
        {
            auto atomsInMiddleRadius = m_roomSimulation->getAtomInRadius( m_roomSimulation->getRoomLen()/2  , false, -75, 150);
            roomAtom* bestAngle = findSpeakerRadius( atomsInMiddleRadius, originalSound, trainer, false, false );
            if ( bestAngle )
                radiusAngle = bestAngle->getInfo().getAngle();
        }


        auto atomsInMiddle = m_roomSimulation->getAtomsInAngle( radiusAngle, 30, false );
        roomAtom* bestRadius = findSpeakerRadius( atomsInMiddle, originalSound, trainerRadius, true );


        soundPositionLocal = bestRadius;
        if ( !soundPositionLocal )
            return;

        radius = bestRadius->getInfo().getRadius();
    }

    if ( !isAngleLocated )
    {
        auto atomInRadius = m_roomSimulation->getAtomInRadius( radius, false, -75, 150);
        soundPositionLocal = findSpeakerRadius(atomInRadius, originalSound, trainer, false );
        if ( !soundPositionLocal )
            return;
        angle = soundPositionLocal->getInfo().getAngle();
    }



    if ( soundPosition && isAngleLocated && radius != -999 )
    {
        if (soundPositionLocal->isAtomRadiusCloseBy( soundPosition, 100 ))
            isRadiusLocated = true;
    }
    if ( soundPosition && angle != -999 && soundPositionLocal->isAtomAngleCloseBy( soundPosition, 10 ) )
    {
        isAngleLocated = true;
    }

    if ( isAngleLocated && isRadiusLocated )
    {
        isSoundLocated = true;
        locationingLogging(input, soundPositionLocal, true, packetCount);
    }
    else
    {
        if ( packetCount >= 10 )
        {
            isSoundLocated = true;
            locationingLogging(input, soundPositionLocal, true, packetCount);
        }
        else
            locationingLogging(input, soundPositionLocal, false, packetCount);
    }

    soundPosition = soundPositionLocal;
}

void roomOracle::locationingLogging( const std::vector< SoundDataRef > &input,
                         roomAtom* bestFinal, bool isFinalized, int packetCount )
{
    if ( packetCount == 1)
    {
        m_resultFile << "**************************************************" << std::endl;
        for ( SoundData<CDataType>& elem : input)
        {
            if ( elem.isSource() )
            {
                m_resultFile << "Original Atom: ";
            }
            m_resultFile << elem.getInfo().get();

        }
    }
    m_resultFile << packetCount << " Is Finalized? " <<  isFinalized << std::endl;
    m_resultFile << "Best Angle,Radius " << angle  <<  "  , "  << bestFinal->getInfo().getRadius() << std::endl;
    pastPositions.push_back(bestFinal->getInfo().getRadius());
    if ( isFinalized )
    {
        for ( SoundData<CDataType>& elem : input)
        {
            if ( elem.isSource() )
            {
                m_resultFile << "Distance: " << elem.getDistance( bestFinal->getInfo().getRealPos() ) << std::endl;
                auto meanRadius = std::accumulate( pastPositions.begin(), pastPositions.end(), 0) / pastPositions.size() ;
                auto meanAtom = m_roomSimulation->findAtomPolarImpl( meanRadius, angle);
                if ( meanAtom )
                {
                    m_resultFile << " Mean Distance: " << elem.getDistance( meanAtom->getInfo().getRealPos() ) << std::endl;
                }
                auto scorerAtom = m_roomSimulation->findAtomPolarImpl( scorer.getBestRadius(), angle);
                if ( scorerAtom )
                {
                    m_resultFile << " Scorer Distance: " << elem.getDistance( scorerAtom->getInfo().getRealPos() ) << std::endl;
                }

                std::sort(pastPositions.begin(), pastPositions.end(), []( double lhs, double rhs ){ return lhs < rhs;});
                double medianRadius = pastPositions[ ((pastPositions.size() - 1) / 2 ) ];
                auto medianAtom = m_roomSimulation->findAtomPolarImpl( medianRadius, angle);
                if ( medianAtom )
                {
                    m_resultFile << " Median Distance: " << elem.getDistance( medianAtom->getInfo().getRealPos() ) << std::endl;
                }


            }
        }
    }

    m_resultFile.flush();

}

void roomOracle::filterByPower(std::vector< roomAtom* >& atomList )
{
    std::sort( atomList.begin(), atomList.end(), []( const roomAtom* lhs, const roomAtom* rhs ){
        return lhs->getResult().second > rhs->getResult().second;
    });

    atomList.resize(30);
}

void roomOracle::postprocess()
{
    m_array->postFiltering(m_weight);
}

roomAtom* roomOracle::findSpeakerRadius( const std::vector< roomAtom* >& atomList,
                                         SoundDataRef originalData,
                                         TrainerComposer& trainerIn,
                                         bool isRadius,
                                         bool isPrint )
{
    (void)(originalData);

    std::vector<double> wholeData(m_packetSize);
    double maxScore = -10000000;
    roomAtom* bestPossibleAtom = nullptr;
    SortedBestPickList bestPicker(10, 3, true);
    for (auto elemGraph : atomList)
    {
        if ( elemGraph->data(0) != "Atom")
            continue;
        auto elem = dynamic_cast<roomAtom*>(elemGraph);
        if ( !elem )
            continue;

        std::fill( wholeData.begin(), wholeData.end(), 0);
        elem->sumWhole(wholeData);
        double curCount = 0;
        double curRatio = 0;
        double curIndexVal = 0;
        if ( IS_SIMPLE_POWER )
        {
            for ( auto elem : wholeData )
                curCount += std::abs(elem);
            curRatio = 1;
            curIndexVal = 1;
        }
        else
        {
            trainerIn.featureCalculation( wholeData  );
            trainerIn.predict( 0 );
            curCount = trainerIn.getRawResult(0);

            if ( isRadius &&  IS_F0_POWER)
            {
                curRatio = 1;
                curIndexVal = 1;
            }
            else
            {
                curIndexVal = trainerIn.getResult(0);
                curRatio = trainerIn.getRatioResult(0);
            }
        }
        trainerIn.clearResults(0);
        auto key = isRadius ? elem->getInfo().getRadius() : elem->getInfo().getAngle();
        bestPicker.insert( key, curCount, curRatio, curIndexVal );
        curCount *= std::pow(curRatio, 1);
        curCount *= std::sqrt(curIndexVal);
        if ( curCount >= maxScore )
        {
            maxScore = curCount;
            if ( curCount > THRESHOLD_POWER   )
                bestPossibleAtom = elem;
        }
    }
    if ( isRadius )
        scorer.feed(bestPicker);
    if (isPrint)
        bestPicker.print();
    return bestPossibleAtom;
}

SoundDataRef roomOracle::feedArray(
        const std::vector< SoundDataRef > &input,
        const CDataType &weights )
{
    SoundDataRef returnVal = input.front();
    if (input.empty())
        std::cout << "RoomOracle <startFeature> no candidate points input was empty" << std::endl;
    for (SoundData<CDataType>& elem : input)
    {
        if ( elem.isSound())
        {
            m_array->adjustArrayFocus( elem.getInfo(), ArrayFocusMode::NO_FOCUS);
            returnVal = elem;
        }
    }

    for (SoundData<CDataType>& elem : input)
    {
        m_array->feed(elem, weights);
    }

    return returnVal;
}

void roomOracle::feedTrainer(const DataConstIter data, int angle)
{
    (void)(data);(void)(angle);
    //m_digger.pushResult(angle, m_trainer.getResult());
}

void roomOracle::getNoice( roomAtom* speakerPos )
{
    int deltaAngle = 10;
    auto dataList = std::move(m_roomSimulation->getArcRadius(speakerPos));
    //radAngDataSummer<DataType> arcRadiusMap;
    radAngDataSummer < std::shared_ptr< DataType > > arcRadiusMap;

    auto speakerAngle = speakerPos->getInfo().getAngle();
    for ( auto& curElem  : dataList.getAllValue() )
    {
        if ( curElem.angle > speakerAngle - deltaAngle && curElem.angle < speakerAngle + deltaAngle  )
            continue;
        DataType wholeData(m_packetSize);
        curElem.data->sumWhole(wholeData);
        SharedDataVec lsharedData = std::make_shared<DataType>(std::move(wholeData));
        arcRadiusMap.insert( curElem.radius, curElem.angle, lsharedData );
    }

    auto sortedAngleEnergy =  std::move (arcRadiusMap.findMax(soundPosition->getInfo().getAngle(), deltaAngle));
    for (auto& elem : sortedAngleEnergy)
    {
        auto existsIter = std::find_if(nullAnglePositions.begin(), nullAnglePositions.end(), [deltaAngle, elem](int i)
        {
            return (elem.first >= i - deltaAngle && elem.first <= i + deltaAngle);
        });

        if (existsIter != nullAnglePositions.end())
        {
            continue;
        }


         nullAnglePositions.push_back(elem.first);

    }

    std::cout << "Noices will be printed out :: " << std::endl;
    for ( size_t i = 0; i < nullAnglePositions.size() && i < 5; i++)
    {
        for ( auto& elem : sortedAngleEnergy )
        {
            if ( elem.first == nullAnglePositions[i])
            {
                std::cout << " Angle: " <<  elem.first << " Ratio: " << elem.second  << std::endl;
                continue;

            }
        }

    }

    nullAnglePositions.resize(1);

}


void
roomOracle::fftWeight()
{
    int N = 360;
//    auto micGainFFT = m_roomSimulation->getImpulseResponce(m_weight);
    std::vector< std::complex<double> > in2 (N, std::complex<double>(0.0, 0));
    auto in2Middle = in2.begin() + in2.size()/2;
    size_t rectLen =  m_array->totalLength() / getWaveLen(1500) *  2;
    std::fill( in2Middle - rectLen/2,
            in2Middle + rectLen/2 + 1, std::complex<double>(1, 0));

    auto micGainFFT = sharpFFT(in2, true);
    micGainFFT = swapVectorWithIn(micGainFFT);

    if ( IS_DRAW)
    {
        auto plot1 = new sharpPlot("Weights1", "Aparture1", false);
        plot1->drawBasicGraph(micGainFFT); plot1->update();

    }
    micGainFFT = swapVectorWithIn(micGainFFT);
    for ( auto angle : nullAnglePositions )
    {
        auto positiveAngle = std::abs(angle) * 2;
        for ( int i = positiveAngle - 10; i < positiveAngle + 11; i++)
        {
            micGainFFT[ N - i ] = 0;
            micGainFFT[ i ] = 0;
        }
    }

    if ( IS_DRAW)
    {
        auto plot4 = new sharpPlot("Weights4", "Apartur4", false);
        plot4->drawBasicGraph(micGainFFT); plot4->update();
    }

    auto result= sharpFFT(micGainFFT, false);

    if ( IS_DRAW)
    {
        auto plot2 = new sharpPlot("Weights2", "Aparture2", false);
        plot2->drawBasicGraph(result); plot2->update();
    }

    auto resMiddlePos = result.begin() + result.size()/2;
    m_weight.assign( resMiddlePos- m_weight.size()/2,
            resMiddlePos + m_weight.size()/2 + 1 );


    auto plot3 = new sharpPlot("Weights3", "Aparture3", false);
    plot3->drawBasicGraph(m_weight); plot3->update();


    m_array->resetBuffers();

}


void roomOracle::parseValidation()
{
    //std::ifstream methodOutput("speakerMethod.txt");
}
