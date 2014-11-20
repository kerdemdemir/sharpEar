#include <audiperiph/audiperiph.h>
#include <vector>
#include <math.h>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <complex>

#define PI (3.1415926535897932384626433832795)

using namespace std;

double HzToERBRate(double Hz)
{
    return( 21.4*log10(Hz*0.00437 + 1.0) );
}

double ERBRateToHz(double ERBRate)
{
    return( (pow(10, ERBRate/21.4) - 1) / 0.00437 );
}

bool Gammatone::initiliaze() {
    // Calculate number of channels, and centre frequencies
    double erb_max = HzToERBRate(max_frequency_);
    double erb_min = HzToERBRate(min_frequency_);
    double delta_erb = (erb_max - erb_min) / (num_channels_ - 1);

    centre_frequencies_.resize(num_channels_);
    double erb_current = erb_min;

    for (int i = 0; i < num_channels_; ++i) {
        centre_frequencies_[i] = ERBRateToHz(erb_current);
        erb_current += delta_erb;
    }

    a_.resize(num_channels_);
    b1_.resize(num_channels_);
    b2_.resize(num_channels_);
    b3_.resize(num_channels_);
    b4_.resize(num_channels_);
    state_1_.resize(num_channels_);
    state_2_.resize(num_channels_);
    state_3_.resize(num_channels_);
    state_4_.resize(num_channels_);

    outputData.init(num_channels_, window_size_, sample_rate_);
    for (int i = 0; i < num_channels_; ++i)
    {
        outputData.set_centre_frequency(i, centre_frequencies_[i]);
    }

    for (int i = 0; i < num_channels_; ++i)
    {
        outputData.set_centre_frequency(i, centre_frequencies_[i]);
    }
    for (int ch = 0; ch < num_channels_; ++ch) {
        double cf = centre_frequencies_[ch];
        double erb = HzToERBRate(cf);

        double dt = 1.0f / sample_rate_;
        double b = 1.019f * 2.0f * M_PI * erb;

        double cpt = cf * M_PI * dt;
        complex<double> exponent(0.0, 2.0 * cpt);
        complex<double> ec = exp(2.0 * exponent);
        complex<double> two_cf_pi_t(2.0 * cpt, 0.0);
        complex<double> two_pow(pow(2.0, (3.0 / 2.0)), 0.0);
        complex<double> p = -2.0 * ec * dt
                           + 2.0 * exp(-(b * dt) + exponent) * dt;
        complex<double> b_dt(b * dt, 0.0);

        double gain = abs(
          (p * (cos(two_cf_pi_t) - sqrt(3.0 - two_pow) * sin(two_cf_pi_t)))
          * (p * (cos(two_cf_pi_t) + sqrt(3.0 - two_pow) * sin(two_cf_pi_t)))
          * (p * (cos(two_cf_pi_t) - sqrt(3.0 + two_pow) * sin(two_cf_pi_t)))
          * (p * (cos(two_cf_pi_t) + sqrt(3.0 + two_pow) * sin(two_cf_pi_t)))
          / pow(-2.0 / exp(2.0 * b_dt) - 2.0 * ec + 2.0 * (1.0 + ec)
                / exp(b_dt), 4.0));

        // The filter coefficients themselves:
        const int coeff_count = 3;
        a_[ch].resize(coeff_count, 0.0f);
        b1_[ch].resize(coeff_count, 0.0f);
        b2_[ch].resize(coeff_count, 0.0f);
        b3_[ch].resize(coeff_count, 0.0f);
        b4_[ch].resize(coeff_count, 0.0f);
        state_1_[ch].resize(coeff_count, 0.0f);
        state_2_[ch].resize(coeff_count, 0.0f);
        state_3_[ch].resize(coeff_count, 0.0f);
        state_4_[ch].resize(coeff_count, 0.0f);

        double B0 = dt;
        double B2 = 0.0f;

        double B11 = -(2.0f * dt * cos(2.0f * cf * M_PI * dt) / exp(b * dt)
                       + 2.0f * sqrt(3 + pow(2.0f, 1.5f)) * dt
                           * sin(2.0f * cf * M_PI * dt) / exp(b * dt)) / 2.0f;
        double B12 = -(2.0f * dt * cos(2.0f * cf * M_PI * dt) / exp(b * dt)
                       - 2.0f * sqrt(3 + pow(2.0f, 1.5f)) * dt
                           * sin(2.0f * cf * M_PI * dt) / exp(b * dt)) / 2.0f;
        double B13 = -(2.0f * dt * cos(2.0f * cf * M_PI * dt) / exp(b * dt)
                       + 2.0f * sqrt(3 - pow(2.0f, 1.5f)) * dt
                           * sin(2.0f * cf * M_PI * dt) / exp(b * dt)) / 2.0f;
        double B14 = -(2.0f * dt * cos(2.0f * cf * M_PI * dt) / exp(b * dt)
                       - 2.0f * sqrt(3 - pow(2.0f, 1.5f)) * dt
                           * sin(2.0f * cf * M_PI * dt) / exp(b * dt)) / 2.0f;;

        a_[ch][0] = 1.0f;
        a_[ch][1] = -2.0f * cos(2.0f * cf * M_PI * dt) / exp(b * dt);
        a_[ch][2] = exp(-2.0f * b * dt);
        b1_[ch][0] = B0 / gain;
        b1_[ch][1] = B11 / gain;
        b1_[ch][2] = B2 / gain;
        b2_[ch][0] = B0;
        b2_[ch][1] = B12;
        b2_[ch][2] = B2;
        b3_[ch][0] = B0;
        b3_[ch][1] = B13;
        b3_[ch][2] = B2;
        b4_[ch][0] = B0;
        b4_[ch][1] = B14;
        b4_[ch][2] = B2;

    }
    return true;
}

std::vector<double> &
Gammatone::startProcess(vector<double>::const_iterator inputData)
{
    process(inputData);
    outputData.calc_power();
    return outputData.getpowerAll();
}

void Gammatone::process(vector<double>::const_iterator inputData)
{
  vector<vector<double> >::iterator b1 = b1_.begin();
  vector<vector<double> >::iterator b2 = b2_.begin();
  vector<vector<double> >::iterator b3 = b3_.begin();
  vector<vector<double> >::iterator b4 = b4_.begin();
  vector<vector<double> >::iterator a = a_.begin();
  vector<vector<double> >::iterator s1 = state_1_.begin();
  vector<vector<double> >::iterator s2 = state_2_.begin();
  vector<vector<double> >::iterator s3 = state_3_.begin();
  vector<vector<double> >::iterator s4 = state_4_.begin();

  // Temporary storage between filter stages
  vector<double> out(window_size_);
  for (int ch = 0; ch < num_channels_;
       ++ch, ++b1, ++b2, ++b3, ++b4, ++a, ++s1, ++s2, ++s3, ++s4) {
    for (int i = 0; i < window_size_; ++i) {
      // Direct-form-II IIR filter
      double in = inputData[i];
      out[i] = (*b1)[0] * in + (*s1)[0];
      for (unsigned int stage = 1; stage < s1->size(); ++stage)
        (*s1)[stage - 1] = (*b1)[stage] * in
                           - (*a)[stage] * out[i] + (*s1)[stage];
    }
    for (int i = 0; i < window_size_; ++i) {
      // Direct-form-II IIR filter
      double in = out[i];
      out[i] = (*b2)[0] * in + (*s2)[0];
      for (unsigned int stage = 1; stage < s2->size(); ++stage)
        (*s2)[stage - 1] = (*b2)[stage] * in
                           - (*a)[stage] * out[i] + (*s2)[stage];
    }
    for (int i = 0; i < window_size_; ++i) {
      // Direct-form-II IIR filter
      double in = out[i];
      out[i] = (*b3)[0] * in + (*s3)[0];
      for (unsigned int stage = 1; stage < s3->size(); ++stage)
        (*s3)[stage - 1] = (*b3)[stage] * in
                           - (*a)[stage] * out[i] + (*s3)[stage];
    }
    for (int i = 0; i < window_size_; ++i) {
      // Direct-form-II IIR filter
      double in = out[i];
      out[i] = (*b4)[0] * in + (*s4)[0];
      for (unsigned int stage = 1; stage < s4->size(); ++stage)
        (*s4)[stage - 1] = (*b4)[stage] * in
                           - (*a)[stage] * out[i] + (*s4)[stage];
      outputData.set_sample(ch, i, out[i]);
    }
  }
}

