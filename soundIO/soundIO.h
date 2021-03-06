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
#include <soxr.h>

inline
int createPulse( CDataType& data, size_t readSize,  double sampleRate )
{
    double f0 = 1500;
    double ts = 1.0 / sampleRate;
    double vz = 500;
    for (size_t i = 0; i < readSize; i++)
    {
        double realTime =  i  * ts;
        double realPart =  cos(2.0*GLOBAL_PI*realTime*f0) *
                    exp(-1.0 * ((i - readSize/2) * (i - readSize/2)) / ( vz * vz));


        data.emplace_back(realPart, 0);

    }

    return data.size();

}


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
        upStreamBufferData.resize( readSize );
        speakerID = -1;
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
        soxResampler = rhs.soxResampler;
        soxDownsampler = rhs.soxDownsampler;
        speakerID = rhs.speakerID;
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

        soxr_error_t error;
        if ( ENABLE_UPSAMPLING )
        {
            auto ioSpec = soxr_io_spec(SOXR_FLOAT64_I,SOXR_FLOAT64_I);
            soxr_quality_spec_t q_spec = soxr_quality_spec(SOXR_VHQ, 0);
            soxResampler = soxr_create( fileInfo->samplerate, fileInfo->samplerate * UP_SAMPLE_RATE, 1,
                                        &error, &ioSpec, &q_spec, NULL);

            soxDownsampler = soxr_create( fileInfo->samplerate * UP_SAMPLE_RATE, fileInfo->samplerate, 1,
                                          &error, &ioSpec, &q_spec, NULL);
        }


        inputStatus = SStatus::JUST_INITED;
        return 0;
    }

    int read( )
    {
        size_t readSizeTemp = readSize;
        if ( ENABLE_UPSAMPLING )
           //readSizeTemp /= (UP_SAMPLE_RATE / 2);
            readSizeTemp /= ( UP_SAMPLE_RATE );

        int readCount = sf_read_double(fileIn, bufferData.data(), readSizeTemp );
        //sf_seek(fileIn, -readSizeTemp/2, SEEK_CUR );
        if ( readCount < 0 )
        {
            std::cout << " IOParams:: <read> failed " << sf_error_number( sf_error(fileIn) ) << std::endl;
            inputStatus = SStatus::FINISHED;
            return -1;
        }

        size_t actualOut;
        if ( ENABLE_UPSAMPLING )
            soxr_process(soxResampler, bufferData.data(), readSizeTemp, NULL, upStreamBufferData.data(), readSize, &actualOut);


        inputStatus = SStatus::ON_GOING;
        if ( ENABLE_UPSAMPLING )
        {
            for (size_t i = 0; i < readSize; i++)
            {
                data.push_back( std::complex<double> (upStreamBufferData[i], 0.0));
            }
        }
        else
        {
            for (size_t i = 0; i < readSize; i++)
                data.push_back( std::complex<double> (bufferData[i], 0.0));
        }

        if ( readCount < (int)readSizeTemp )
        {
            std::cout << " IOParams:: <read> File comes to end " << readCount << " Total Read " << totalRead << std::endl;
            inputStatus = SStatus::FINISHED;
            return 0;
        }
        //std::cout << " IOParams:: <read>  Read succeed count " << readCount << " Total Read " << totalRead << std::endl;
        return readCount;
    }

    int pulse(double sampleRate)
    {
        return createPulse( data, readSize, sampleRate);
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
       sf_count_t count = 0;
       if (ENABLE_UPSAMPLING)
       {
            std::vector<double> copy (data.size() / UP_SAMPLE_RATE ) ;
            soxr_process(soxDownsampler, data.data(), data.size(), NULL, copy.data(), copy.size(), NULL);
            count = sf_write_double( fileOut, copy.data(), copy.size()) ;
       }
        else
             count = sf_write_double( fileOut, data.data(), data.size()) ;
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


    size_t getSpeakerID() const
    {
    return speakerID;
    }

    void setSpeakerID(const size_t &value)
    {
    speakerID = value;
    }

private:

    static std::vector<double> bufferData;
    static std::vector<double> upStreamBufferData;
    CDataType data;
    std::string filename;
    SNDFILE* fileIn ;
    SNDFILE* fileOut;
    std::unique_ptr <SF_INFO> fileInfo;
    size_t readSize;
    SStatus inputStatus;
    int totalRead;
    int totalWrite;
    soxr_t soxResampler;
    soxr_t soxDownsampler;
    size_t speakerID;
};


#endif // SOUNDIO_H


