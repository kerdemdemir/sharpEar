//Copyright (c) 2014,
//Kadir Erdem Demir
//All rights reserved.

//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. All advertising materials mentioning features or use of this software
//   must display the following acknowledgement:
//   This product includes software developed by the <organization>.
//4. Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY Kadir Erdem Demir ''AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef AUDIPERIPH_H
#define AUDIPERIPH_H

#include <vector>
#include <iostream>
#include <algorithm>

#define MINCF			80
#define MAXCF			8000


double HzToERBRate(double Hz);
double ERBRateToHz(double ERBRate);

class SignalBank {

public:

    bool init(int channel_count, int signal_length, float sample_rate)
    {
        sample_rate_ = sample_rate;
        buffer_length_ = signal_length;
        channel_count_ = channel_count;
        signals_.resize(channel_count_);
        centre_frequencies_.resize(channel_count_, 0.0f);
        for (int i = 0; i < channel_count_; ++i)
        {
          signals_[i].resize(buffer_length_, 0.0f);
        }
        initialized_ = true;
        return true;
    }

    float sample(int channel, int index) const {
      return signals_[channel][index];
    }

    void set_sample(int channel, int index, float value) {
      signals_[channel][index] = value;
    }

    float sample_rate() const {
      return sample_rate_;
    }

    int buffer_length() const {
      return buffer_length_;
    }

    float centre_frequency(int i) const {
      if (i < channel_count_)
        return centre_frequencies_[i];
      else
        return 0.0f;
    }

    void set_centre_frequency(int i, float cf) {
      if (i < channel_count_)
        centre_frequencies_[i] = cf;
    }

    bool initialized() const {
      return initialized_;
    }

    int channel_count() const {
      return channel_count_;
    }

    void calc_power()
    {
        powers.resize(channel_count_);
        for (int k = 0; k < channel_count_; k++)
        {
            powers[k] = 0;
            for (int j = 0; j < buffer_length_; j++)
            {
                 powers[k] += signals_[k][j] * signals_[k][j];
            }
            //perfectiveLoudless(k);
            powers[k] = log10(powers[k]);
        }
    }

    void perfectiveLoudless(int channel)
    {
        double omega = centre_frequencies_[channel] * 3.147 * 2;
        double million = 1000000;
        double numerator = (pow(omega, 2.0) + 56.8 * million)* pow(omega,4.0);
        double denomirator = pow((pow(omega, 2) + 6.3* million),2.0) * (pow(omega,2.0) + 38*million);
        double E = numerator / denomirator;
        powers[channel] = E * powers[channel];
    }

    double get_power(int channel)
    {
        return powers[channel];
    }

    std::vector<double>& getpowerAll()
    {
        return powers;
    }

private:

    int channel_count_;
    int buffer_length_;
    std::vector<double> centre_frequencies_;
    std::vector<double> powers;
    float sample_rate_;
    bool initialized_;
    std::vector<std::vector<double> > signals_;

};




class Gammatone
{
public:
    bool init(int numberOfChannels, float sampleRate, size_t packetSize)
    {
        num_channels_ = numberOfChannels;
        max_frequency_ = MAXCF;
        min_frequency_ = MINCF;
        window_size_ = 20 * sampleRate / 1000;;
        sample_rate_ = sampleRate;
        packetSize_ = packetSize;
        return initiliaze();
    }
    std::vector<double>& startProcess(const std::vector<double>::const_iterator inputData);

    bool avaliable(size_t curPos)
    {
        return ((curPos + window_size_) < packetSize_);
    }

private:

    void process(const std::vector<double>::const_iterator inputData);
    void reverse();
    bool initiliaze();

    // Filter coefficients
    std::vector<std::vector<double> > b1_;
    std::vector<std::vector<double> > b2_;
    std::vector<std::vector<double> > b3_;
    std::vector<std::vector<double> > b4_;
    std::vector<std::vector<double> > a_;

    std::vector<std::vector<double> > state_1_;
    std::vector<std::vector<double> > state_2_;
    std::vector<std::vector<double> > state_3_;
    std::vector<std::vector<double> > state_4_;

    std::vector<double> centre_frequencies_;
    SignalBank outputData;
    float sample_rate_;
    int num_channels_;
    int window_size_;
    int window_count_;
    double max_frequency_;
    double min_frequency_;
    size_t packetSize_;
};

#endif // AUDIPERIPH_H

