#ifndef AMPLITUDESUM
#define AMPLITUDESUM

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>


class AmplitudeSum : public FeatureExtractor
{
public:

    AmplitudeSum( int formant )
    {
        samples = cv::Mat(  1, 1 , CV_64FC1 );
    }

    ~AmplitudeSum()
    {
    }



    virtual DataType2D& getFeatures() override
    {
        return samples;
    }


    virtual void doChunk( fvec_t *inputSimple, cvec_t *inputComplex ) override
    {
        auto result = 0.0;
        for ( auto i = 0; i < inputSimple->length; i++)
            result += inputSimple->data[i];

        samples.at<double>(colSize, 0) = result;
        colSize++;
    }

private:

};

#endif // AMPLITUDESUM

