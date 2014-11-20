#ifndef AUDIPERIPH_H
#define AUDIPERIPH_H

#include <deque>
#include <vector>
#include <iostream>
#include <array>
#include <map>

#define SAMPLING_FREQUENCY		44000			// Sampling frequency
#define MINCF			80
#define MAXCF			5000


double HzToERBRate(double Hz);
double ERBRateToHz(double ERBRate);


struct soundChannel4GammoTone
{
    double cf, bw;					// Center frequency and bandwidth
    double midEarCoeff;				// Midear coefficient, which normalizing the amplitude of the filter response
    double p[4];						// The real part of the filter response
    double q[4];						// The imaginary part of the filter response
    double delay;					// The average delay of the filter
};

class AudiPeriph
{

public :

    AudiPeriph()
    {


    }

    void startCochleagram(const std::vector<double>::const_iterator paramSoundRawData, size_t sizeOfInput, int paramNumberOfChannels, int samplingFrequency);

    std::map<int, std::map<int, std::vector<double> > >& getChannel2WindowOutputMap();

    std::map<int, std::map<int, double> >& getChannel2WindowEnergyMap() ;

    double getMaxEnergy();

private:

    void prepareGammatoneStruct();
    void gammaToneFilter(int channelNumber);
    void cochleagram();

    void setInputData(const std::vector<double>::const_iterator paramSoundRawData) const;
    mutable std::vector<double>::const_iterator inputData;
    size_t _sizeOfInput;
    std::vector<soundChannel4GammoTone> gammoToneChannelVec;
    std::vector <std::vector<double> > gammoOutput;
    std::vector <std::vector<double> > hairCellOutput;
    std::map < int, std::map<int, std::vector<double> > > channel2WindowOutputMap;
    std::map < int, std::map<int, double > > channel2WindowEnergyMap;
    int numberOfChannels;
    double samplingFrequency;
    int winLength;
    int winShift;
    int numberOfWindows;
    double maxEnergy;
};


#endif // AUDIPERIPH_H

