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

#include <vector>
#include <string>
#include <complex>
#include <QPoint>
#include <iostream>



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
