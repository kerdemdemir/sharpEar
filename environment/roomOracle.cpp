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
        return;

    //m_weight.resize(m_array.getElemCount());
    m_weight.resize(m_array.getElemCount(), 1);
    //fftWeight(SoundInfo(),  0 );
    //m_weight.resize(m_array.getElemCount(), 1);
    feedArray(input, m_weight);

    auto atomsInMiddle = m_roomSimulation->getAtomsInAngle( 0 );
    roomAtom* bestRadius = findSpeakerRadius( atomsInMiddle, trainer );
    auto atomInRadius = m_roomSimulation->getAtomInRadius( bestRadius->getInfo().getRadius());
    auto soundPositionLocal = findSpeakerRadius(atomInRadius, trainer );
    std::cout << " Speaker located in: ";
    soundPositionLocal->print();
    if ( soundPositionLocal->isAtomRadiusCloseBy( soundPosition, 100 ))
    {
        isSoundLocated = true;
    }
    else
    {
        soundPosition = soundPositionLocal;
        return;
    }
    getNoice( );

}


roomAtom* roomOracle::findSpeakerRadius( const std::vector< roomAtom* >& atomList,
                                         TrainerComposer& trainerIn )
{
    std::vector<double> wholeData(m_packetSize);
    int maxScore = 0;
    roomAtom* bestPossibleAtom;
    for (auto elem : atomList)
    {
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

void roomOracle::feedArray(const std::vector< SoundDataRef > &input, const CDataType &weights )
{
    if (input.empty())
        std::cout << "RoomOracle <startFeature> no candidate points input was empty" << std::endl;
    for (SoundData<CDataType>& elem : input)
    {
        if ( elem.isSound())
            m_array.adjustArrayFocus( elem.getInfo(), ArrayFocusMode::NO_FOCUS);
    }

    for (SoundData<CDataType>& elem : input)
    {
        m_array.feed(elem, weights);
    }
}

void roomOracle::feedTrainer(const DataConstIter data, int angle)
{
    //m_digger.pushResult(angle, m_trainer.getResult());
}

void roomOracle::getNoice( )
{
    int deltaAngle = 10;
    auto dataList = std::move(m_roomSimulation->getArcRadius(soundPosition));
    //radAngDataSummer<DataType> arcRadiusMap;
    radAngDataSummer < std::shared_ptr< DataType > > arcRadiusMap;

    DataType wholeData(m_packetSize);
    for ( auto atom : dataList.getAllData() )
    {
        atom->sumWhole(wholeData);
        SharedDataVec lsharedData = std::make_shared<DataType>(std::move(wholeData));
        arcRadiusMap.insert( atom->getInfo().getRadius(), atom->getInfo().getAngle(), lsharedData );
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
            std::cout << " Digger <startDigging>  Angle " << elem.first << " is extension of angle "
                      <<  *existsIter << " it won't be taken to account " << std::endl;
            continue;
        }

        if (elem.second > 0.25)
        {
            nullAnglePositions.push_back(elem.first);
        }
        else
            break;
    }

}


void
roomOracle::fftWeight()
{
    int N = 180;
    std::vector< std::complex<double> > in2 (N, std::complex<double>(0, 0));



    auto middlePos = in2.begin() + in2.size()/2;
    std::fill( middlePos - m_weight.size()/2, middlePos + m_weight.size()/2 + 1, std::complex<double>(1, 0)  ) ;
    auto micGainFFT = sharpFFT(in2, true);


    auto result= sharpFFT(micGainFFT, false);
    auto resMiddlePos = result.begin() + result.size()/2;
    m_weight.assign( resMiddlePos- m_weight.size()/2,
            resMiddlePos + m_weight.size()/2 + 1 );
}


void roomOracle::parseValidation()
{
    //std::ifstream methodOutput("speakerMethod.txt");
}
