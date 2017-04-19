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

#ifndef MY_UTILITY_H
#define MY_UTILITY_H

#define _USE_MATH_DEFINES

#include <vector>
#include <string>
#include <complex>
#include <QPoint>
#include <iostream>
#include "utility/types.h"
#include <fftw3.h>
#include <math.h>
//#include <Eigen/Core>
//#include <unsupported/Eigen/FFT>

#define M_PI           3.14159265358979323846

inline
void sharpWindow( CDataType& in )
{
    for (size_t i = 0; i < in.size() ; i++)
    {
        int j = i - in.size();
        in[i] = (in[i] * 0.5 * (1.0 - cos(2.0 * M_PI * j / in.size())));
    }
}

inline
double sharpSinc(double x) {

    if(x == 0.0)
        return 1.0;
    return std::sin(x)/x;
}

inline
double getWaveLen(double frequencyHz )
{
    return 34000.0/frequencyHz;
}

inline
void resizeInterpolate( CDataType& in, size_t newSize)
{
    (void)(in);(void)(newSize);
    //if ()
}

inline
CDataType sharpFFT( CDataType& in, bool isForward )
{
    const int N =  in.size() ;

    std::vector< std::complex<double> > out (N);

    fftw_plan my_plan;
    if (!isForward)
      my_plan = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(&in[0]),
                                             reinterpret_cast<fftw_complex*>(&out[0]), FFTW_BACKWARD, FFTW_ESTIMATE);
    else
      my_plan = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(&in[0]),
                                               reinterpret_cast<fftw_complex*>(&out[0]), FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(my_plan);
    if (!isForward)
    {
        std::transform(out.begin(), out.end(), out.begin(), [N]( std::complex<double>& elem ){
            return elem / (double)N;
        });
    }
    return out;
}

inline
CDataType sharpFFT( CDataType::iterator in, size_t size, bool isForward )
{
    const int N =  size ;

    std::vector< std::complex<double> > out (size);

    fftw_plan my_plan;
    if (!isForward)
      my_plan = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(&in[0]),
                                             reinterpret_cast<fftw_complex*>(&out[0]), FFTW_BACKWARD, FFTW_ESTIMATE);
    else
      my_plan = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(&in[0]),
                                               reinterpret_cast<fftw_complex*>(&out[0]), FFTW_FORWARD, FFTW_ESTIMATE);

    fftw_execute(my_plan);


    return out;
}

template< typename T >
inline
T swapVectorWithIn( const T& in )
{

    auto middlePos = in.begin() + in.size()/2;
    T returnVal( middlePos , in.end() );
    returnVal.insert( returnVal.end(), in.begin(), middlePos);
    return returnVal;
}

inline
CDataType sharpIFFT( DataType in )
{
    const int N =  in.size() + 1 ;

    std::vector< std::complex<double> > out (N);

    std::vector< std::complex<double> > returnVal (N);

    fftw_plan my_plan = fftw_plan_dft_r2c_1d(N,
                        reinterpret_cast<double*>(&in[0]),
                        reinterpret_cast<fftw_complex*>(&out[0]),
                        FFTW_ESTIMATE);

    fftw_execute(my_plan);
    auto outMiddle = out.begin() + out.size()/2;
    returnVal.assign(outMiddle, out.end());
    returnVal.insert( returnVal.end(), out.begin() + 1, outMiddle);

    return returnVal;
}

template< typename T>
double calculateSNR( const T& originalData, const T& compareData  )
{
    if ( originalData.size() != compareData.size() )
        std::cout << " SnrManager: Calculate SNR fails sizes are different ";

    double val;
    double noice;
    for ( int i = 0; i < originalData.size(); i++ )
    {
         noice += std::pow(  std::abs(originalData[i] - compareData[i]), 2 );
         val += std::pow(  std::abs(originalData[i]), 2 );
    }

    return noice/val;

}


struct QpointHash
{
    std::size_t operator()(const QPoint& k) const
    {
        return k.x() * 900 + k.y();
    }
    //int m_multiplier;
};

struct QpointEqual
{
    bool operator()(const QPoint& lhs, const QPoint& rhs) const
    {
        return lhs.x() == rhs.x() && lhs.y() == rhs.y();
    }
};

bool operator <(QPoint point1, QPoint point2);


double prepareSNR(const std::vector<double>& a, const double soundSourceSum);
double randomGenerator();
void writeToFile(const std::vector<double> &refVector);
void writeToFile(const char* value);
void openFileIO();
void closeFile();

template < class... Args >
void writeToFileStr(const char* fmt, const Args ...args)
{
    char buff[4096];
    snprintf(buff, 4096, fmt, args...);
    writeToFile(buff);

}


#endif
