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

#ifndef TYPES_H
#define TYPES_H

#include <complex>
#include <vector>
#include <utility>
#include <functional>
#include <memory>
#include <map>

using Point = std::pair<int, int>;
using DPoint = std::pair<double, double>;
using SingleCDataType = std::complex<double>;
using CDataType = std::vector< std::complex<double> > ;
using DataType = std::vector< double > ;
using CDataIter = std::vector<std::complex<double>>::iterator;
using DataIter = std::vector<double>::iterator;
using DataConstIter = std::vector<double>::const_iterator;
using CDataConstIter = std::vector<std::complex<double>>::const_iterator;
using SharedDataVec = std::shared_ptr<DataType>;

template<class T>
using SharedData = std::shared_ptr<T>;
template<class T>
using ref_t = std::reference_wrapper<T>;



#endif // TYPES_H
