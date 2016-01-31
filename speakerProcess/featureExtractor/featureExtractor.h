#ifndef DEFINITIONS_H
#define DEFINITIONS_H


#include <string>
#include <vector>
#include "speakerProcess/general.h"
#include <aubio.h>




class FeatureExtractor
{
public:

    virtual void doChunk( fvec_t *inputSimple, cvec_t *inputComplex ) = 0;
    virtual DataType2D& getFeatures() = 0;

    void filefinished()
    {
        samples.resize(colSize);
        colSize = 0;
    }


    size_t colSize = 0;
    DataType2D samples;

};





#endif // DEFINITIONS_H
