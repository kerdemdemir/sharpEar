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
        selectedFormant = formant;

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
        (void)(inputSimple);
        double freqStep = sampleRate / win_s;

        int startFreq = 1000 * selectedFormant;
        size_t startIndex = startFreq/ freqStep;
        size_t stopIndex = ((selectedFormant + 2) * 1000) / freqStep;
        stopIndex = std::min( stopIndex, inputComplex->length -1 );

        auto result = 0.0;
        while( startIndex != stopIndex  )
        {
            result +=  inputComplex->norm[ startIndex++ ];
        }
        samples.at<double>(colSize, 0) = result;
        colSize++;
    }

private:

    int selectedFormant;

};

#endif // AMPLITUDESUM

