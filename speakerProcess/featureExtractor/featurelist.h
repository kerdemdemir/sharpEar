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

    void addExtractor( std::shared_ptr<FeatureExtractor> extractor )
    {
        extractors.push_back( extractor );
    }

private:
    fvec_t *inputSimple;
    aubio_pvoc_t *phaseVocedor;
    cvec_t *inputComplex;
    std::vector< std::shared_ptr<FeatureExtractor> > extractors;

};

#endif // FEATURELIST_H

