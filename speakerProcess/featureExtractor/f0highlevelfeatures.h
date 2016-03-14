#ifndef F0HIGHLEVELFEATURES_H
#define F0HIGHLEVELFEATURES_H

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>

class F0HighLevelFeatures : public FeatureExtractor
{
public:

    F0HighLevelFeatures( int startFormantIn, int formantCountIn )
    {
        formantCount = startFormantIn;
        startCount = formantCountIn;
        char cStr[] = "default";

        samples = cv::Mat(  1, formantCount , CV_64FC1 );

        pitchOut = new_fvec (1); // output candidate
        pitch = new_aubio_pitch (cStr, win_s, hopSize, sampleRate);
    }

    ~F0HighLevelFeatures()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    void getFormants( double f0, cvec_t* inputComplex )
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

       for ( int i = 0; i < formantCount; i++)
       {
            samples.at<double>(colSize, i) = formants[startCount + i].first / JUMPSIZE ;
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

    int formantCount;
    int startCount;
    aubio_pitch_t *pitch;
    fvec_t *pitchOut;
};

#endif // F0HIGHLEVELFEATURES_H

