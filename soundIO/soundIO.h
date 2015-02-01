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

#ifndef SOUNDIO_H
#define SOUNDIO_H

#include <sndfile.h>
#include <vector>
#include <utility/commons.h>
#include <string>
#include <memory>
#include <math.h>

class IOParams
{
public:
    IOParams (size_t packetSize) : fileInfo(new SF_INFO)
    {
        fileIn = nullptr;
        fileOut = nullptr;
        fileInfo->format = 0;
        totalRead = 0;
        totalWrite = 0;
        readSize = packetSize;
        bufferData.resize(readSize);
        inputStatus = SStatus::NOT_INIT;
    }

    IOParams() = default;

    IOParams( IOParams&& rhs)
    {
        fileIn = rhs.fileIn;
        fileOut = rhs.fileOut;
        fileInfo = std::move(rhs.fileInfo);
        totalRead = rhs.totalRead;
        totalWrite = rhs.totalWrite;
        readSize = rhs.readSize;
        inputStatus = rhs.inputStatus;

        rhs.fileIn = nullptr;
        rhs.fileOut = nullptr;
    }

    ~IOParams ()
    {
        if (fileIn != nullptr)
            sf_close(fileIn);
        if (fileOut != nullptr)
            sf_close(fileOut);
    }

    bool isRead()
    {
        return !data.empty();
    }

    CDataIter getDataIter ()
    {
        if (data.size() < readSize)
        {
            std::cout << " IOParams <getLastData> : dataSize is smaller than readSize "  << std::endl;
            return data.end();
        }
        return (data.end() - readSize);
    }

    CDataIter getDataEnd ()
    {
        return data.end();
    }

    int open(const std::string& fileName, bool isRead)
    {
        if (isRead)
            fileIn = sf_open(fileName.c_str(), SFM_READ , fileInfo.get());
        else
            fileOut = sf_open(fileName.c_str(), SFM_WRITE, fileInfo.get());

        SNDFILE* file = (isRead ? fileIn : fileOut);
        if ( sf_error (file)  != 0)
        {
            std::cout << "IOParams:: <open> failed " << sf_error_number( sf_error(file) ) << " isRead: " << isRead << std::endl;
            return -1;
        }
        std::cout << " Sample rate: " <<  fileInfo->samplerate << " Channels: " << fileInfo->channels << std::endl;
        inputStatus = SStatus::JUST_INITED;
        return 0;
    }

    int read( )
    {
        int readCount = sf_read_double(fileIn, bufferData.data(), readSize);
        if ( readCount < 0 )
        {
            std::cout << " IOParams:: <read> failed " << sf_error_number( sf_error(fileIn) ) << std::endl;
            inputStatus = SStatus::FINISHED;
            return -1;
        }
        inputStatus = SStatus::ON_GOING;
        for (size_t i = 0; i < readSize; i++)
            data.push_back( std::complex<double> (bufferData[i], 0.0));

        totalRead = data.size();

        if ( readCount < (int)readSize )
        {
            std::cout << " IOParams:: <read> File comes to end " << readCount << " Total Read " << totalRead << std::endl;
            inputStatus = SStatus::FINISHED;
            return 0;
        }
        std::cout << " IOParams:: <read>  Read succeed count " << readCount << " Total Read " << totalRead << std::endl;
        return readCount;
    }

    int pulse(double sampleRate)
    {
        double f0 = 2000;
        double ts = 1.0 / sampleRate;
        double vz = 100;
        for (size_t i = 0; i < readSize; i++)
        {
            double realTime =  i  * ts;
            double realPart =  cos(2.0*GLOBAL_PI*realTime*f0) *
                        exp(-1.0 * ((i - readSize/2) * (i - readSize/2)) / ( vz * vz));
            double imagePart = sin(2.0*GLOBAL_PI*realTime*f0) *
                        exp(-1.0 * ((i - readSize/2) * (i - readSize/2)) / ( vz * vz));

            data.emplace_back(realPart, imagePart);

        }
        return data.size();
    }

    bool isWritten()
    {
        if (fileOut != nullptr)
            return true;
        return false;

    }

    int write(const std::vector<double> &data)
    {
       if ( fileOut == NULL )
       {
            std::cout << "IOParams:: <write> fileOut is null please check it" << std::endl;
            return -1;
       }

        sf_count_t count = sf_write_double( fileOut, data.data(), data.size()) ;
        std::cout << "IOParams:: <write> Data size written: " << count << std::endl;

        if (count <= 0)
        {
            std::cout << "IOParams:: <write> Error with data size written: " <<  std::endl;
            return -1;
        }

        return count;
    }

    SStatus getReadStatus()
    {
        return inputStatus;
    }

    SF_INFO& getInfo()
    {
        return *fileInfo.get();
    }

    void setInfo(SF_INFO& info)
    {
        fileInfo.get()->channels = info.channels;
        fileInfo.get()->format = info.format;
        fileInfo.get()->frames = info.frames;
        fileInfo.get()->samplerate = info.samplerate;
        fileInfo.get()->sections = info.sections;
        fileInfo.get()->seekable = info.seekable;
    }


private:

    static std::vector<double> bufferData;
    CDataType data;
    std::string filename;
    SNDFILE* fileIn ;
    SNDFILE* fileOut;
    std::unique_ptr <SF_INFO> fileInfo;
    size_t readSize;
    SStatus inputStatus;
    int totalRead;
    int totalWrite;
};


#endif // SOUNDIO_H
