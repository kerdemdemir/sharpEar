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

SortedBestPickList curBestPicker;

roomOracle::roomOracle(size_t sampleRate, size_t packetSize, int speakerID, int noiceID, microphoneNode &array): scorer(6000, 25)
{
    m_array = &array;
    m_sampleSize = sampleRate;
    m_packetSize = packetSize;
    m_speakerID = speakerID;
    m_noiceID = noiceID;

    std::string trainPath("D:/speakerWavs/train1");
    trainer.initPGrams(0, "SilenceRemove2GramDefaultAubio");
    //trainer.initPWave(0, "WaveF0");

    //trainer.initYINPGrams(0, "F0YinFFT2Gram");
    //trainer.initYINPGrams(0, "F0MultiYinFFT2Gram");
    //3Gramtrainer.initPGrams(0, "F0YinFFT");
    //trainer.initPGrams(0, "F0YinFFT2Gram");
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
    SoundData<CDataType>& originalSound = feedArray(input, m_weight);
    std::cout << " Is Radius Located " << isRadiusLocated << "Is Angle Located " << isAngleLocated << std::endl;
    if ( packetCount == 0  )
         return;
    if ( !isRadiusLocated )
    {
        double radiusAngle = angle;
        if ( angle == -999 )
        {
            auto atomsInMiddleRadius = m_roomSimulation->getAtomInRadius( m_roomSimulation->getRoomLen()/3  , true,  -88, 176);
            auto atomsInMiddleRadius3 = m_roomSimulation->getAtomInRadius( m_roomSimulation->getRoomLen()*2/3  , true, -90, 180);
            atomsInMiddleRadius.insert( atomsInMiddleRadius.end(), atomsInMiddleRadius3.begin(), atomsInMiddleRadius3.end());

            soundPositionLocal = findSpeaker( atomsInMiddleRadius, originalSound, trainer, false, true );
            if ( soundPositionLocal )
                radiusAngle = soundPositionLocal->getInfo().getAngle();
        }


        auto atomsInMiddle = m_roomSimulation->getAtomsInAngle( radiusAngle, 20, true );
        soundPositionLocal = findSpeaker( atomsInMiddle, originalSound, trainer, true,true );
        if ( soundPositionLocal )
            radius = soundPositionLocal->getInfo().getRadius();
        else
            radius = m_roomSimulation->getRoomLen()/2;
    }

    if ( !isAngleLocated )
    {
        auto atomInRadius = m_roomSimulation->getAtomInRadius( radius, true, -88, 176);
        soundPositionLocal = findSpeaker(atomInRadius, originalSound, trainer, false, true );
        if ( soundPositionLocal )
           angle = soundPositionLocal->getInfo().getAngle();
        else
           angle = 0;
    }

    if ( soundPosition && soundPositionLocal )
    {
        if (soundPositionLocal->isAtomRadiusCloseBy( soundPosition, 200 ))
            isRadiusLocated = true;
    }
    if ( soundPosition && soundPositionLocal && soundPositionLocal->isAtomAngleCloseBy( soundPosition, 10 ) )
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
            m_resultFile << elem.getString();

        }
    }

    m_resultFile << packetCount << " Is Finalized? " <<  isFinalized << std::endl;
    if (!bestFinal)
        return;
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


roomAtom*
roomOracle::findSpeaker(const std::vector< roomAtom* >& atomList,
                                         SoundData<CDataType>& originalData,
                                         TrainerComposer& trainerIn,
                                         bool isRadius,
                                         bool isPrint )
{
    auto speakers = findSpeakers(atomList, originalData, trainerRadius, isRadius );
    return findBestSpeaker(speakers, originalData, trainerIn, isRadius, isPrint );

}

std::vector<roomAtom*>
roomOracle::findSpeakers(const std::vector< roomAtom* >& atomList,
                                         SoundData<CDataType>& originalData,
                                         TrainerComposer& trainerIn,
                                         bool isRadius )
{
    if (atomList.empty())
        return std::vector<roomAtom*>();

    std::vector<roomAtom*> returnVal;
    std::vector<double> wholeData(m_packetSize);
    SortedBestPickList bestPicker(5, 8, true);

    for (auto elem : atomList)
    {
        if ( elem == nullptr)
            continue;
        std::fill( wholeData.begin(), wholeData.end(), 0);
        elem->sumWhole(wholeData);
        double curCount = 0;
//        for ( auto elem : wholeData )
//            curCount += std::abs(elem);
        trainerIn.featureCalculation( wholeData  );
        trainerIn.predict( originalData.getSpeakerID() );
        curCount = trainerIn.getRawResult(originalData.getSpeakerID());
        trainerIn.clearResults(originalData.getSpeakerID());
        auto key = isRadius ? elem->getInfo().getRadius() : elem->getInfo().getAngle();
        bestPicker.insert( key, curCount, 1, 1 );
    }
    std::cout <<  " Eliminating step is complete: " << std::endl;
    bestPicker.print();

    roomAtom* curAtom = nullptr;
    for ( auto elem : bestPicker.getPairList() )
    {
        auto constantVal = isRadius ? atomList.front()->getInfo().getAngle() : atomList.front()->getInfo().getRadius();
        for ( int i = -2; i < 3; i++ )
        {
            auto offsetedConstantVal = constantVal - (isRadius ? i * 10 : i) ;
            if ( isRadius )
            {
                curAtom = m_roomSimulation->findAtomPolarImpl(elem.first, offsetedConstantVal);
            }
            else
            {
                curAtom = m_roomSimulation->findAtomPolarImpl(offsetedConstantVal, elem.first);
            }
            returnVal.push_back( curAtom );
        }

    }

    return returnVal;
}

roomAtom*
roomOracle::findBestSpeaker(const std::vector< roomAtom* >& atomList,
                                         SoundData<CDataType>& originalData,
                                         TrainerComposer& trainerIn,
                                         bool isRadius,
                                         bool isPrint )
{
    std::vector<double> wholeData(m_packetSize);
    double maxScore = -10000000;
    roomAtom* bestPossibleAtom = nullptr;
    SortedBestPickList bestPicker(10, 0, true);
    int counter = 0;
    for (auto elem : atomList)
    {
        counter++;
        if ( elem == nullptr)
            continue;
        std::fill( wholeData.begin(), wholeData.end(), 0);
        elem->sumWhole(wholeData);
        trainerIn.featureCalculation( wholeData  );
        trainerIn.predict( originalData.getSpeakerID() );
        double curRatio = trainerIn.getRatioResult(originalData.getSpeakerID());
        double curVal = trainerIn.getNormalizedRawResult(originalData.getSpeakerID());
        trainerIn.clearResults(originalData.getSpeakerID());
        auto key = isRadius ? elem->getInfo().getRadius() : elem->getInfo().getAngle();
        auto lastVal = ((curVal * curRatio * curRatio ) / sqrt(sqrt(counter)) ) ;
        bestPicker.insert( key, curVal, curRatio, lastVal );

        if ( lastVal >= maxScore )
        {
            maxScore = lastVal;
            bestPossibleAtom = elem;
        }

    }
    std::cout <<  " Speaker selection step is complete: " << std::endl;

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
        if ( elem.isSource())
        {
            returnVal = elem;
        }
        m_array->adjustArrayFocus( elem.getInfo(), ArrayFocusMode::NO_FOCUS);
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
