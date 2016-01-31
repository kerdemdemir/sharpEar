#ifndef TRANIERLIST
#define TRANIERLIST

#include "modelbase.h"
#include "speakerProcess/featureExtractor/featurelist.h"
#include "speakerProcess/featureExtractor/f0features.h"
#include "speakerProcess/featureExtractor/MFCCFeatures.h"
#include "speakerProcess/mlModel/gmmModel.h"
#include "speakerProcess/mlModel/pitchgrams.h"
#include "speakerProcess/mlModel/tranierlist.h"
#include <QElapsedTimer>

class TrainerComposer : public ModelBase
{
public:

    TrainerComposer()
    {
        modelName = "Fusion";
    }

    void featureCalculation ( const DataType& input )
    {
        featureList.start(input);
    }

    virtual void predict( int personID )
    {
        QElapsedTimer timer;
        for ( auto model : modelList)
        {
            try
            {
                timer.restart();
                model->predict( personID );
                model->predictionTime += timer.elapsed();
            }
            catch ( ... )
            {
                std::cout << "Exp caught while predicting " << std::endl;
                continue;
            }
        }
    }

    virtual void feed( const std::string& fileName )
    {
        //if (featureList.start(fileName) == -1)
          //  return;

        for ( auto model : modelList)
        {
            //if ( !model->isLoad )
              //  model->predict( fileName );
        }
    }

    virtual void train ()
    {
        QElapsedTimer timer;
        for ( auto model : modelList)
        {
            try
            {
                timer.restart();
                if ( !model->isLoad )
                {
                    model->train();
                    model->save();
                }
                model->miliSecTrainTime += timer.elapsed();
            }
            catch ( ... )
            {
                std::cout << "Exp caught while training " << std::endl;
                continue;
            }
        }
    }

    virtual void save()
    {
        for ( auto model : modelList)
            model->save();
    }

    virtual bool load()
    {
        bool isAllLoaded = true;
        for ( auto model : modelList)
        {
            if ( model->isLoad )
                model->load();
            else
                isAllLoaded = false;
        }
        return isAllLoaded;
    }

    resultType getResults()
    {
        return std::move(modelList[0]->speakerResultList);
    }

    void addModel( std::shared_ptr<ModelBase> model )
    {
        modelList.push_back( model );
    }

    void initPGrams( int selectedGram, std::string selectedGramName )
    {
        auto F0FeaturePtr = std::make_shared<F0Features>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(3, selectedGramName);
        pitchGramRunnerModel->isLoad = true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void init()
    {
//        auto mfccFeaturePtr = std::make_shared<MFCCFeatures>();
//        auto gmmModel = std::make_shared<GMMModel>("MFCC");
//        featureList.addExtractor(mfccFeaturePtr);
//        gmmModel->setFeature( mfccFeaturePtr );
//        //gmmModel->isLoad = true;
//        addModel(gmmModel);

        auto F0FeaturePtr = std::make_shared<F0Features>(0);
        //auto gmmF0Model = std::make_shared<GMMModel>("Formant", 4);
        featureList.addExtractor(F0FeaturePtr);
        //gmmF0Model->setFeature( F0FeaturePtr );
        //gmmF0Model->isLoad = true;
        //addModel(gmmF0Model);


        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(3, "F0");
        pitchGramRunnerModel->isLoad = true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    std::vector<std::shared_ptr<ModelBase>>&
    getModels()
    {
        return modelList;
    }

private:

    FeatureList featureList;
    std::vector<std::shared_ptr<ModelBase>> modelList;
};

inline
void train( TrainerComposer& trainer, const std::string& testFilePath )
{

    if ( trainer.load() )
        return;

    for (auto& elem : getFileNames(testFilePath))
    {
        std::cout << "Traning: " << elem.first << std::endl;
        trainer.feed(elem.first);
    }
    trainer.train();


    std::cout << "Training is over " << std::endl;
}


#endif // TRANIERLIST

