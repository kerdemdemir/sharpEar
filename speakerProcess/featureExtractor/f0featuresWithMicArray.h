#ifndef __F0FeaturesMicArray
#define __F0FeaturesMicArray

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>
#include "dywapitchtrack.h"

#define IS_WAVELET 0

class F0FeaturesMicArray : public FeatureExtractor
{
public:

    F0FeaturesMicArray( int formant )
    {
        selectedFormant = formant;
        char cStr[] = "default";
        samples = cv::Mat(  1, 1 , CV_64FC1 );
        pitchOut = new_fvec (1); // output candidate
        pitch = new_aubio_pitch (cStr, win_s, hopSize, sampleRate);

        dywapitch_inittracking(&pitchtracker);

        aubio_pitch_set_tolerance (pitch, 0.95);
        aubio_pitch_set_silence (pitch, -30);
    }

    ~F0FeaturesMicArray()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    void getFormants( double f0, cvec_t* inputComplex )
    {
       double freqStep = sampleRate / win_s;
       std::array< std::pair<size_t, double> , FORMANT_COUNT> formants{ std::make_pair(0.0, 0.0 ) };

       size_t formantIndex = f0 / freqStep;
       formants[0].second = inputComplex->norm[formantIndex];

       for ( int curFreq = f0; curFreq < 7000; curFreq += f0 )
       {
            if ( curFreq < 1000 && curFreq >= 8000  )
                continue;

            int formant = curFreq / 1000;
            formantIndex = curFreq / freqStep;
            if ( formantIndex >= inputComplex->length )
                continue;
            float curFormantVal = inputComplex->norm[formantIndex];
            formants[formant].first++;
            formants[formant].second += curFormantVal;
       }

       if ( !formants[selectedFormant].first )
       {
            samples.at<double>(colSize, 0) = 0;
            return;
       }

       double meanVal = formants[selectedFormant].second / formants[selectedFormant].first;
       auto result =  meanVal; // * (IS_WAVELET ? 1.0 : aubio_pitch_get_confidence(pitch));
       samples.at<double>(colSize, 0) = result;

    }

    virtual DataType2D& getFeatures() override
    {
        return samples;
    }


    virtual void doChunk( fvec_t *inputSimple, cvec_t *inputComplex ) override
    {
        if ( IS_WAVELET )
        {
            double thepitch = dywapitch_computepitch(&pitchtracker, (double*)inputSimple->data, 0, inputSimple->length);
            thepitch *= ((double)sampleRate / 44100.0);
            if ( thepitch < MIN_FREQ || thepitch > MAX_FREQ )
                return;
            getFormants( thepitch, inputComplex);
        }
        else
        {
            aubio_pitch_do (pitch, inputSimple, pitchOut);
            if ( pitchOut->data[0] < MIN_FREQ || pitchOut->data[0] > MAX_FREQ )
                return;
            getFormants( pitchOut->data[0], inputComplex);

        }
        colSize++;
    }

private:

    int selectedFormant;
    aubio_pitch_t *pitch;
    fvec_t *pitchOut;
    dywapitchtracker pitchtracker;

};

#endif // F0FEATURES

