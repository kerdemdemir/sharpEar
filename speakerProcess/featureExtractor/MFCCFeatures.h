#ifndef MFCCFEATUREEXTRACTOR
#define MFCCFEATUREEXTRACTOR

#include "featureExtractor.h"

#define NUMBER_OF_MFCC_FEATURES 39
#define NUMBER_OF_MFCC_FILTERS 40


class MFCCFeatures : public FeatureExtractor
{
public:

    MFCCFeatures()
    {
        samples = cv::Mat(  1, NUMBER_OF_MFCC_FEATURES , CV_64FC1 );
        mfccout = new_fvec (NUMBER_OF_MFCC_FEATURES); // output candidates
        mfcc = new_aubio_mfcc (win_s, NUMBER_OF_MFCC_FILTERS, NUMBER_OF_MFCC_FEATURES, sampleRate);
    }

    ~MFCCFeatures()
    {
        del_aubio_mfcc (mfcc);
        del_fvec (mfccout);
        aubio_cleanup ();
    }

    virtual DataType2D& getFeatures() override
    {
        return samples;
    }


    virtual void doChunk(  fvec_t *inputSimple, cvec_t *inputComplex ) override
    {
        aubio_mfcc_do( mfcc, inputComplex, mfccout );

        for ( int i = 0; i < NUMBER_OF_MFCC_FEATURES; i++ )
            samples.at<double>(colSize, i) = mfccout->data[i];
        colSize++;
    }

private:

    fvec_t *mfccout;
    aubio_mfcc_t *mfcc;

};


#endif // MFCCFEATUREEXTRACTOR

