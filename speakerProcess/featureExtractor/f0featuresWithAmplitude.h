#ifndef F0FEATURESWITHAMPLITUDE
#define F0FEATURESWITHAMPLITUDE

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>


class F0FeaturesAmplitude : public FeatureExtractor
{
public:

    F0FeaturesAmplitude( int formant )
    {
        selectedFormant = formant;
        char cStr[] = "default";
        samples = cv::Mat(  1, 2 , CV_64FC1 );

        pitchOut = new_fvec (1); // output candidate
        pitch = new_aubio_pitch (cStr, win_s, hopSize, sampleRate);
        aubio_pitch_set_silence (pitch, -30);
    }

    ~F0FeaturesAmplitude()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    void getFormants( double f0, cvec_t* inputComplex )
    {
       double freqStep = (sampleRate/2) / win_s;
       std::array< std::pair<double, double> , FORMANT_COUNT> formants;
       formants[0].first = f0;
       int formantIndex = formants[0].first / freqStep;
       formants[0].second = inputComplex->norm[formantIndex];

       if ( selectedFormant == 0 )
       {
           samples.at<double>(colSize, 0) = (f0 - MIN_FREQ) / 10;
           samples.at<double>(colSize, 1) = formants[0].second;//aubio_pitch_get_confidence(pitch);
           return;
       }

       int startFreq = 1000 * selectedFormant;
       size_t startIndex = startFreq/ freqStep;
       size_t stopIndex = ((selectedFormant + 1) * 1000) / freqStep;
       stopIndex = std::min( stopIndex, inputComplex->length -1 );

       double result = -1000000;
       size_t bestIndex = startIndex;
       size_t curIndex = startIndex;
       while( curIndex != stopIndex  )
       {
           if (  inputComplex->norm[ curIndex ]  > result)
           {
               result = inputComplex->norm[ curIndex ];
               bestIndex = curIndex;
           }
           curIndex++;
       }

       samples.at<double>(colSize, 0) = freqStep*(bestIndex-startIndex) / 25.0;
       samples.at<double>(colSize, 1) = result;
    }

    virtual DataType2D& getFeatures() override
    {
        return samples;
    }


    virtual void doChunk( fvec_t *inputSimple, cvec_t *inputComplex ) override
    {
        aubio_pitch_do (pitch, inputSimple, pitchOut);

        if ( pitchOut->data[0] < MIN_FREQ || pitchOut->data[0] > MAX_FREQ )
            return;

        getFormants( pitchOut->data[0], inputComplex);
        colSize++;
    }

private:

    int selectedFormant;
    double amplitudeOfFormant;
    aubio_pitch_t *pitch;
    fvec_t *pitchOut;
};

#endif // F0FEATURES

