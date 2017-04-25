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
        char cStr[] = "yin";
        samples = cv::Mat(  1, 2 , CV_64FC1 );

        pitchOut = new_fvec (1); // output candidate
        pitch = new_aubio_pitch (cStr, win_s, hopSize, sampleRate);
    }

    ~F0FeaturesAmplitude()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    void getFormants( double f0, cvec_t* inputComplex )
    {
       double freqStep = sampleRate / win_s;
       std::array< std::pair<double, double> , FORMANT_COUNT> formants;
       formants[0].first = f0;
       int formantIndex = formants[0].first / freqStep;
       formants[0].second = inputComplex->norm[formantIndex];

       if ( selectedFormant == 0 )
       {
           samples.at<double>(colSize, 0) = (f0 - MIN_FREQ) / 10;
           samples.at<double>(colSize, 1) = formants[0].second * aubio_pitch_get_confidence(pitch);
           return;
       }

       for ( int curFreq = f0; curFreq < FORMANT_COUNT * 1000; curFreq += f0 )
       {
            if ( curFreq < 1000 )
            {
                continue;
            }
            int formant = curFreq / 1000;
            int formantIndex = curFreq / freqStep;
            float curFormantVal = inputComplex->norm[formantIndex];
            if ( formants[formant].second < curFormantVal )
            {
                formants[formant].first = curFreq % 1000;
                formants[formant].second = curFormantVal;
            }
       }
       if ( selectedFormant != -1 )
       {
           samples.at<double>(colSize, 0) = (formants[selectedFormant].first) / 40;//JUMPSIZE*2 ;// / (JUMPSIZE * 5);
           samples.at<double>(colSize, 1) = formants[selectedFormant].second * aubio_pitch_get_confidence(pitch);
       }
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

