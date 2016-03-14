#include <environment/roomOracle.h>
#include <sharpplot.h>
#include <speakerProcess/mlModel/tranierlist.h>
#include "environment/roomSimulation.h"
#include "environment/roomAtom.h"
#include "utility/utility.h"

constexpr int IS_DRAW = 1;


void roomOracle::preprocess(const std::vector< SoundDataRef > &input )
{
    if ( isSoundLocated )
    {
        feedArray(input, m_weight);
        return;
    }
    m_weight.resize(m_array.getElemCount(), 1);
    //feedArray(input, m_weight); return;
    if ( isManualMode )
    {
        fftWeight();
        auto soundPos = feedArray(input, m_weight);
        isSoundLocated = true;
        SoundData<CDataType>& dataOriginal = soundPos;
        auto atomInRadius = m_roomSimulation->findAtomPolarImpl(dataOriginal.getInfo().getRadius(), dataOriginal.getInfo().getAngle());
        auto snrVal = dataOriginal.calculateSNR(atomInRadius->sumWhole());
        dataOriginal.getInfo().print( QString(" SNR val : %1 ").arg(snrVal).toStdString());
        return;
    }

    auto originalSound = feedArray(input, m_weight);

    auto atomsInMiddle = m_roomSimulation->getAtomsInAngle( 0 );
    roomAtom* bestRadius = findSpeakerRadius( atomsInMiddle, originalSound, trainer4 );
    auto atomInRadius = m_roomSimulation->getAtomInRadius( bestRadius->getInfo().getRadius(), false);
    auto soundPositionLocal = findSpeakerRadius(atomInRadius, originalSound, trainer4 );
    std::cout << " Speaker located in: ";
    SoundData<CDataType>& dataOriginal = originalSound;
    auto snrVal = dataOriginal.calculateSNR(soundPositionLocal->sumWhole());
    soundPositionLocal->getInfo().print( QString(" SNR val : %1 ").arg(snrVal).toStdString());
    soundPosition = soundPositionLocal;
    isSoundLocated = true;
    if ( soundPositionLocal->isAtomRadiusCloseBy( soundPosition, 100 ))
    {
        isSoundLocated = true;
    }
    else
    {
        soundPosition = soundPositionLocal;
        return;
    }
    getNoice( soundPositionLocal );
    fftWeight();

    m_array.resetBuffers();
    originalSound = feedArray(input, m_weight);
    std::cout << " After weighting: ";
    snrVal = dataOriginal.calculateSNR(soundPositionLocal->sumWhole());
    soundPositionLocal->getInfo().print( QString(" SNR val : %1 ").arg(snrVal).toStdString());
}


roomAtom* roomOracle::findSpeakerRadius( const std::vector< roomAtom* >& atomList,
                                         SoundDataRef originalData,
                                         TrainerComposer& trainerIn )
{
    std::vector<double> wholeData(m_packetSize);
    int maxScore = 0;
    roomAtom* bestPossibleAtom;
    for (auto elem : atomList)
    {
        std::fill( wholeData.begin(), wholeData.end(), 0);
        if ( elem->isNearField )
            continue;
        elem->sumWhole(wholeData);

        trainerIn.featureCalculation( wholeData  );
        trainerIn.predict( 0 );
        auto results = trainerIn.getResults();
        auto curCount = 0;

        for ( scoreType& holderList : results[0] )
        {
            auto scoreIndex = sortIndexes<NUMBER_OF_PEOPLE>(holderList);
            auto score = std::distance ( scoreIndex.begin(), std::find(scoreIndex.begin(), scoreIndex.end(), 0 ));
            curCount += score;
        }

        if ( curCount >= maxScore )
        {
            maxScore = curCount;
            bestPossibleAtom = elem;
        }

    }
    return bestPossibleAtom;
    //return elem->getInfo().getRadius();;

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
            m_array.adjustArrayFocus( elem.getInfo(), ArrayFocusMode::NO_FOCUS);
            returnVal = elem;
        }
    }

    for (SoundData<CDataType>& elem : input)
    {
        m_array.feed(elem, weights);
    }

    return returnVal;
}

void roomOracle::feedTrainer(const DataConstIter data, int angle)
{
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
    nullAnglePositions.resize(2);

}


void
roomOracle::fftWeight()
{
    int N = 360;
//    auto micGainFFT = m_roomSimulation->getImpulseResponce(m_weight);
    std::vector< std::complex<double> > in2 (N, std::complex<double>(0.0, 0));
    auto in2Middle = in2.begin() + in2.size()/2;
    std::fill( in2Middle - m_weight.size()/2,
            in2Middle + m_weight.size()/2 + 1, std::complex<double>(1, 0));

    auto micGainFFT = sharpFFT(in2, true);
    micGainFFT = swapVectorWithIn(micGainFFT);
    auto plot1 = new sharpPlot("Weights1", "Aparture1", false);
    plot1->drawBasicGraph(micGainFFT); plot1->update();
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
    auto plot4 = new sharpPlot("Weights4", "Apartur4", false);
    plot4->drawBasicGraph(micGainFFT); plot4->update();

    auto result= sharpFFT(micGainFFT, false);
    auto plot2 = new sharpPlot("Weights2", "Aparture2", false);
    plot2->drawBasicGraph(result); plot2->update();

    auto resMiddlePos = result.begin() + result.size()/2;
    m_weight.assign( resMiddlePos- m_weight.size()/2,
            resMiddlePos + m_weight.size()/2 + 1 );


    auto plot3 = new sharpPlot("Weights3", "Aparture3", false);
    plot3->drawBasicGraph(m_weight); plot3->update();
    m_array.resetBuffers();

}


void roomOracle::parseValidation()
{
    //std::ifstream methodOutput("speakerMethod.txt");
}
