#ifndef TRANIERLIST
#define TRANIERLIST

#include "modelbase.h"
#include "speakerProcess/featureExtractor/featurelist.h"
#include "speakerProcess/featureExtractor/f0features.h"
#include "speakerProcess/featureExtractor/f0featuresWithAmplitude.h"
#include "speakerProcess/featureExtractor/f0featuresWithMicArray.h"
#include "speakerProcess/featureExtractor/MFCCFeatures.h"
#include "speakerProcess/featureExtractor/pyinf0feature.h"
#include "speakerProcess/featureExtractor/PYINFeatureMicArray.h"
#include "speakerProcess/featureExtractor/amplitudesum.h"
#include "speakerProcess/mlModel/gmmModel.h"
#include "speakerProcess/mlModel/pitchgrams.h"
#include "speakerProcess/mlModel/tranierlist.h"
#include "speakerProcess/mlModel/simpleSummerModel.h"
#include "speakerProcess/featureExtractor/f0highlevelfeatures.h"
#include "speakerProcess/featureExtractor/wavef0withamplitude.h"
#include "speakerProcess/featureExtractor/peakFrequency.h"
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

    void featureBasicFFTCalculation ( const DataType& input )
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
                model->predict( personID );
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
        if (featureList.start(fileName) == -1)
            return;

        for ( auto model : modelList)
        {
            if ( !model->isLoad )
                model->feed( fileName );
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

    virtual void load()
    {
        for ( auto model : modelList)
        {
            if ( model->isLoad )
                model->load();
        }
    }

    bool isAllLoaded()
    {
        bool isAllLoaded = true;
        for ( auto model : modelList)
        {
            if ( !model->isLoad )
                isAllLoaded = false;
        }
        return isAllLoaded;
    }

    scoreList getResults( int id )
    {
        speakerResultList[id].clear();
        for ( auto model : modelList)
        {
            for ( size_t i = 0; i < model->speakerResultList[id].size(); i++)
            {
                if ( speakerResultList[id].size() <= i  )
                    speakerResultList[id].push_back( sortIndexesDouble<NUMBER_OF_PEOPLE>(model->speakerResultList[id][i]) );
                else
                {
                    auto curResult = sortIndexesEqually<NUMBER_OF_PEOPLE>(model->speakerResultList[id][i]);
                    for ( int k = 0; k < NUMBER_OF_PEOPLE; k++)
                    {
                        speakerResultList[id][i][k] += curResult[k];
                    }
                }
            }

            model->speakerResultList[id].clear();
        }
        return  speakerResultList[id];
    }

    void clearResults( int id )
    {
        for ( auto model : modelList)
        {
            model->speakerResultList[id].clear();
        }
        featureList.clear();
    }

    int getResult( int id )
    {
        int result = 0;
        for ( auto model : modelList)
        {
            for ( size_t i = 0; i < model->speakerResultList[id].size(); i++)
            {
                result += sortIndexesEqually<NUMBER_OF_PEOPLE>(model->speakerResultList[id][i])[id] ;
            }

            //model->speakerResultList[id].clear();
        }
        return  result;
    }

    double getNormalizedRawResult( int id )
    {
        double result = 0;
        for ( auto model : modelList)
        {
            double curChunkSize = model->speakerResultList[id].size();
            for ( size_t i = 0; i < model->speakerResultList[id].size(); i++)
            {
                result += model->speakerResultList[id][i][id] / curChunkSize ;
            }
            //model->speakerResultList[id].clear();
        }
        return  result / modelList.size();
    }


    double getRawResult( int id )
    {
        double result = 0;
        for ( auto model : modelList)
        {
            for ( size_t i = 0; i < model->speakerResultList[id].size(); i++)
            {
                result += model->speakerResultList[id][i][id] ;
            }
            //model->speakerResultList[id].clear();
        }
        return  result;
    }

    double getRatioResult( int id )
    {
        double result = 0;
        for ( auto model : modelList)
        {
            double curChunkSize = model->speakerResultList[id].size();
            for ( size_t i = 0; i < model->speakerResultList[id].size(); i++)
            {
                auto totalSum = std::accumulate( model->speakerResultList[id][i].begin(), model->speakerResultList[id][i].end(), 0.0 );
                result += ((model->speakerResultList[id][i][id] / totalSum) / curChunkSize) ;
            }
        }
        return  result / modelList.size();
    }

    void addModel( std::shared_ptr<ModelBase> model )
    {
        modelList.push_back( model );
    }

    void initPGrams( int selectedGram, std::string selectedGramName, bool isLoad )
    {
        auto F0FeaturePtr = std::make_shared<F0FeaturesAmplitude>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(2, selectedGramName);
        pitchGramRunnerModel->isLoad = isLoad;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void initPRadiusGrams( int selectedGram, std::string selectedGramName, bool isLoad )
    {
        auto F0FeaturePtr = std::make_shared<F0FeaturesMicArray>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(2, selectedGramName);
        pitchGramRunnerModel->isLoad = isLoad;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void initPeakFreq( int selectedGram, std::string selectedGramName, bool isLoad )
    {
        auto F0FeaturePtr = std::make_shared<FreqeuncyPeak>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(2, selectedGramName);
        pitchGramRunnerModel->isLoad = isLoad;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }


    void initP0Power( int selectedGram )
    {
        auto F0FeaturePtr = std::make_shared<AmplitudeSum>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchSummer = std::make_shared<SummerModel>();
        pitchSummer->setFeature( F0FeaturePtr );
        addModel(pitchSummer);
    }

    void initPWave( int selectedGram, std::string selectedGramName )
    {
        auto F0FeaturePtr = std::make_shared<WaveF0WithAmplitude>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(3, selectedGramName);
        pitchGramRunnerModel->isLoad =  true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);

    }

    void initP0Grams(  )
    {
        int selectedGram = 0; std::string selectedGramName = "F0";
        auto F0FeaturePtr = std::make_shared<F0FeaturesAmplitude>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(1, selectedGramName);
        pitchGramRunnerModel->isLoad =  true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void initYINPGrams( int selectedGram, std::string selectedGramName )
    {
        auto F0FeaturePtr = std::make_shared<PYINF0>(selectedGram);
        featureList.addExtractor(F0FeaturePtr);
        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(2, selectedGramName);
        pitchGramRunnerModel->isLoad = true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void initAmplitudeSum( )
    {

    }

    void initHighLevelGMM()
    {
        auto highLevelFeaturePtr = std::make_shared<F0HighLevelFeatures>(4,2);
        auto gmmF0Model = std::make_shared<GMMModel>("HighLevelFormant");
        featureList.addExtractor(highLevelFeaturePtr);
        gmmF0Model->setFeature( highLevelFeaturePtr );
        gmmF0Model->isLoad = true;
        addModel(gmmF0Model);
    }

    void initMFCC()
    {
        auto mfccFeaturePtr = std::make_shared<MFCCFeatures>();
        auto gmmModel = std::make_shared<GMMModel>("MFCC");
        featureList.addExtractor(mfccFeaturePtr);
        gmmModel->setFeature( mfccFeaturePtr );
        gmmModel->isLoad = true;
        addModel(gmmModel);
    }

    void initFusion()
    {
        auto mfccFeaturePtr = std::make_shared<MFCCFeatures>();
        auto gmmModel = std::make_shared<GMMModel>("MFCC");
        featureList.addExtractor(mfccFeaturePtr);
        gmmModel->setFeature( mfccFeaturePtr );
        gmmModel->isLoad = true;
        addModel(gmmModel);

        auto F0FeaturePtr = std::make_shared<F0Features>(-1);
        auto gmmF0Model = std::make_shared<GMMModel>("Formant");
        featureList.addExtractor(F0FeaturePtr);
        gmmF0Model->setFeature( F0FeaturePtr );
        gmmF0Model->isLoad = true;
        addModel(gmmF0Model);

//        auto highLevelFeaturePtr = std::make_shared<F0HighLevelFeatures>(2,2);
//        auto gmmF0Model = std::make_shared<GMMModel>("HighLevelFormant");
//        featureList.addExtractor(highLevelFeaturePtr);
//        gmmF0Model->setFeature( highLevelFeaturePtr );
//        gmmF0Model->isLoad = false;
//        addModel(gmmF0Model);


        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(3, "F0");
        pitchGramRunnerModel->isLoad = true;
        pitchGramRunnerModel->setFeature( F0FeaturePtr );
        addModel(pitchGramRunnerModel);
    }

    void init()
    {
        auto F0FeaturePtr = std::make_shared<F0Features>(4);
        //auto gmmF0Model = std::make_shared<GMMModel>("Formant", 4);
        featureList.addExtractor(F0FeaturePtr);
        //gmmF0Model->setFeature( F0FeaturePtr );
        //gmmF0Model->isLoad = true;
        //addModel(gmmF0Model);


        auto pitchGramRunnerModel = std::make_shared<PitchGramModel>(3, "F4");
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

    trainer.load();
    if ( trainer.isAllLoaded() )
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

