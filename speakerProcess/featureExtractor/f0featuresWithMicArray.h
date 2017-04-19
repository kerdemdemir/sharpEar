#ifndef __F0FeaturesMicArray
#define __F0FeaturesMicArray

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>
#include "environment/microphonenode.h"
#include "environment/roomAtom.h"
#include "DspFilters/Filter.h"
#include "DspFilters/Dsp.h"

class F0FeaturesMicArray : public FeatureExtractor
{
public:

    F0FeaturesMicArray( int formant )
    {
        selectedFormant = formant;
        char cStr[] = "default";

        if ( selectedFormant != -1 )
            samples = cv::Mat(  1, 1 , CV_64FC1 );
        else
            samples = cv::Mat(  1, FORMANT_COUNT , CV_64FC1 );

        pitchOut = new_fvec (1); // output candidate
        pitch = new_aubio_pitch (cStr, win_s, hopSize, sampleRate);
    }

    ~F0FeaturesMicArray()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    double getVal( )
    {
        return totalValue;
    }

    double getFormants( double f0, cvec_t* inputComplex )
    {
       double freqStep = sampleRate / win_s;
       std::array< std::pair<double, double> , FORMANT_COUNT> formants;
       formants[0].first = f0;
       for ( int curFreq = f0; curFreq < 8000; curFreq += f0 )
       {
            if ( curFreq < 1000 )
                continue;

            int formant = curFreq / 1000;
            int formantIndex = curFreq / freqStep;
            float curFormantVal = inputComplex->norm[formantIndex];
            if ( formants[formant].second < curFormantVal )
            {
                formants[formant].first = curFreq % 1000;
                formants[formant].second = curFormantVal;
            }
       }


       return formants[selectedFormant].second;
    }

    virtual DataType2D& getFeatures() override
    {
        return samples;
    }


    virtual void doChunk( fvec_t *inputSimple, cvec_t *inputComplex ) override
    {
        if ( colSize == 0 )
            totalValue = 0;
        aubio_pitch_do (pitch, inputSimple, pitchOut);
        if ( pitchOut->data[0] < MIN_FREQ || pitchOut->data[0] > MAX_FREQ )
            return;
        colSize++;
        totalValue += getFormants( pitchOut->data[0], inputComplex);
    }

private:

    double totalValue;
    int selectedFormant;
    double amplitudeOfFormant;
    aubio_pitch_t *pitch;
    fvec_t *pitchOut;
};

#endif // F0FEATURES

