#ifndef F0FEATURES
#define F0FEATURES

#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include <aubio.h>

constexpr int FORMANT_COUNT = 8;
constexpr int STATE_COUNT =  ((MAX_FREQ - MIN_FREQ) / JUMPSIZE);

class F0Features : public FeatureExtractor
{
public:

    F0Features( int formant )
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

    ~F0Features()
    {
        del_aubio_pitch (pitch);
        del_fvec (pitchOut);
    }

    void getFormants( double f0, cvec_t* inputComplex )
    {
       double freqStep = sampleRate / win_s;
       std::array< std::pair<double, double> , FORMANT_COUNT> formants;
       formants[0].first = f0;
       for ( int curFreq = f0; curFreq < FORMANT_COUNT * 1000; curFreq += f0 )
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
       if ( selectedFormant != -1 )
       {
           if ( selectedFormant == 0 )
            samples.at<double>(colSize, 0) = (formants[selectedFormant].first - MIN_FREQ) / 25;
           else
            samples.at<double>(colSize, 0) = (formants[selectedFormant].first) / JUMPSIZE*2 ;// / (JUMPSIZE * 5);
       }
       else
       {
           for ( int i = 0; i < FORMANT_COUNT; i++)
           {
                if ( i ==  0)
                    samples.at<double>(colSize, i) = (formants[i].first - MIN_FREQ) / JUMPSIZE ;
                else
                    samples.at<double>(colSize, i) = (formants[i].first - MIN_FREQ) / (JUMPSIZE * 2);
           }
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

