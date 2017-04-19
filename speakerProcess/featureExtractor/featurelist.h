#ifndef FEATURELIST_H
#define FEATURELIST_H

#include <aubio.h>
#include <sndfile.h>
#include <iostream>
#include <fstream>
#include <range/v3/all.hpp>
#include "speakerProcess/featureExtractor/featureExtractor.h"
#include "speakerProcess/general.h"
#include "utility/commons.h"
#include "utility/soundData.h"


struct Reader
{
    static constexpr size_t MAX_FILE_SIZE = 4000000;


    using ReaderIter = std::vector< double >::iterator;
    using ReaderView = ranges::counted_view< ReaderIter, int >;

    std::vector<double> rawData;

    std::array<size_t, NUMBER_OF_PEOPLE> trainSizes;
    size_t trainUpLimit;

    size_t fileSize;

    Reader () : trainSizes{0}
    {
        trainUpLimit = 500000000;
        rawData.resize(MAX_FILE_SIZE);
    }

    ReaderView getView()
    {
        return ranges::counted_view< ReaderIter, int >( rawData.begin(), fileSize);
    }

    ReaderView
    openWavFile(std::string fileName)
    {
        SF_INFO soundFileInfo;
        SNDFILE* soundFile ;

        int state = fileName2State(fileName);
        if ( trainSizes[state] >  trainUpLimit )
        {
            fileSize = 0;
            std::cout << "Up Limit for speaker " << state  << std::endl;
            return getView();
        }

        soundFileInfo.format = 0;
        soundFile = sf_open(fileName.c_str(), SFM_READ , &soundFileInfo);


        if ( sf_error (soundFile)  != 0)
        {
            std::cout << "Problem with input wav file" << std::endl;
            fileSize = 0;
        }

        fileSize = sf_read_double(soundFile, rawData.data(), MAX_FILE_SIZE);
        //for ( size_t i = 0; i < fileSize; i++)
            // rawData[i] *= 3.3;

        if (fileSize <= 0)
        {
            std::cout << "Problem while reading the file" << std::endl;
            fileSize = 0;
        }

        trainSizes[state] += fileSize / (sampleRate / 1000) ;

        return getView();
    }
};

class FeatureList
{
public:

    FeatureList()
    {
        inputSimple = new_fvec(hopSize);
        phaseVocedor = new_aubio_pvoc( win_s, hopSize);
        inputComplex = new_cvec(win_s);
    }

    ~FeatureList()
    {
        del_cvec (inputComplex);
        del_fvec(inputSimple);
        del_aubio_pvoc(phaseVocedor);
    }

    int start ( const DataType& input )
    {
        auto readerChunks =  ranges::view::all(input) | ranges::view::chunk(hopSize);

        for ( auto extractor : extractors )
            extractor->getFeatures().resize( readerChunks.size() );

        RANGES_FOR( auto chunk, readerChunks )
        {
            copyRangeToArray( chunk, inputSimple->data );
            aubio_pvoc_do( phaseVocedor, inputSimple, inputComplex );
            for ( auto extractor : extractors )
                extractor->doChunk( inputSimple, inputComplex );
        }

        for ( auto extractor : extractors )
            extractor->filefinished();

        return 0;
   }

    int start ( const std::string& input )
    {
        auto readView = reader.openWavFile(input);

        if ( readView.empty())
            return -1;

        auto readerChunks = readView | ranges::view::chunk(hopSize);

        for ( auto extractor : extractors )
            extractor->getFeatures().resize( readerChunks.size() );

        RANGES_FOR( auto chunk, readerChunks )
        {
            copyRangeToArray( chunk, inputSimple->data );
            aubio_pvoc_do( phaseVocedor, inputSimple, inputComplex );
            for ( auto extractor : extractors )
                extractor->doChunk( inputSimple, inputComplex );
        }

        for ( auto extractor : extractors )
            extractor->filefinished();

        return 0;
   }

    void addExtractor( std::shared_ptr<FeatureExtractor> extractor )
    {
        extractors.push_back( extractor );
    }

private:

    Reader reader;
    fvec_t *inputSimple;
    aubio_pvoc_t *phaseVocedor;
    cvec_t *inputComplex;
    std::vector< std::shared_ptr<FeatureExtractor> > extractors;

};

#endif // FEATURELIST_H

