#ifndef TRAINER_H
#define TRAINER_H

#include <map>
#include "opencv2/ml.hpp"
#include "opencv2/core.hpp"
#include <speakerProcess/featureExtractor/featureExtractor.h>
#include <algorithm>
#include <range/v3/all.hpp>
#include <QString>
#include "modelbase.h"

class GMMModel : public ModelBase
{
public:
    GMMModel( std::string name, int clusterNumber = 5 )
    {
        modelName = name;
        sampleList.resize(NUMBER_OF_PEOPLE);
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            auto emPtr = cv::ml::EM::create();
            emPtr->setClustersNumber(clusterNumber);
            gmmModelVec.push_back(emPtr);
        }
    }


    virtual void feed( const std::string& fileName ) override
    {
        int state = fileName2State(fileName);
        if ( sampleList[state].empty() )
            sampleList[state] = features->getFeatures();
        else
            cv::vconcat(sampleList[state], features->getFeatures(), sampleList[state]);
    }

    virtual void train() override
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            gmmModelVec[i]->trainEM(sampleList[i]);
        }
    }

    virtual void save() override
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            cv::FileStorage fileHolder( QString("Speaker_%1_%2").arg(modelName.c_str()).arg(QString::number(i)).toStdString(), cv::FileStorage::WRITE );
            gmmModelVec[i]->write(fileHolder);
        }
    }

    virtual void load() override
    {
        for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
        {
            cv::FileStorage fileHolder( QString("Speaker_%1_%2").arg(modelName.c_str()).arg(QString::number(i)).toStdString(), cv::FileStorage::READ );
            if (fileHolder.isOpened())
            {
                const cv::FileNode& fn = fileHolder["StatModel.EM"];
                gmmModelVec[i]->read(fn);
                fileHolder.release();
            }
        }
    }

    virtual void predict( int personID ) override
    {
        std::array<double,NUMBER_OF_PEOPLE> tempHolder{0};
        for ( size_t k = 0; k < features->colSize; k++ )
        {
            for ( int i = 0; i < NUMBER_OF_PEOPLE; i++)
            {
                tempHolder[i] = gmmModelVec[i]->predict2(features->getFeatures().row(k), cv::noArray())[0];
            }
            // auto scores = sortIndexes<double, NUMBER_OF_PEOPLE>(tempHolder);
            //auto distance = std::distance ( tempHolder.begin(), std::max_element(tempHolder.begin(), tempHolder.end()));
            if ( !isAllEqual(tempHolder) )
             speakerResultList[personID].push_back(  tempHolder );
        }
    }


private:

    std::vector< cv::Ptr<cv::ml::EM> > gmmModelVec;
    std::vector< cv::Mat > sampleList;
};

#endif // TRAINER_H
