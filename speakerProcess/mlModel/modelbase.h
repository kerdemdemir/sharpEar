#ifndef MODELBASE
#define MODELBASE

#include "speakerProcess/general.h"
#include <QTime>

class FeatureExtractor;

using scoreType = std::array<double,NUMBER_OF_PEOPLE>;
using scoreList = std::vector< scoreType >;
using resultType = std::array< scoreList, NUMBER_OF_PEOPLE>;

class ModelBase
{

public:
    virtual void predict( int personID ) = 0;
    virtual void feed ( const std::string& fileName ) = 0;
    virtual void train () = 0;
    virtual void save() = 0;
    virtual bool load() = 0;

    void setFeature( std::shared_ptr<FeatureExtractor> featurePtr )
    {
        features = featurePtr;
    }

    bool isLoad = false;


    size_t miliSecTrainTime = 0 ;
    size_t predictionTime = 0;
    std::string modelName;
    resultType speakerResultList;
    std::shared_ptr<FeatureExtractor> features;
};



#endif // MODELBASE

