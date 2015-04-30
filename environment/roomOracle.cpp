#include <environment/roomOracle.h>
#include <sharpplot.h>

void roomOracle::preprocess(const std::vector<ref_t<SoundData<CDataType> > > &input )
{
   fftWeight();
   //m_weight.resize(m_array.getElemCount(), 1);
   feedArray(input, m_weight);
}

void roomOracle::postprocess(const std::vector<ref_t<SoundData<CDataType> > > &input)
{
    feedArray(input, m_weight);
}

void roomOracle::feedArray(const std::vector<ref_t<SoundData<CDataType> > > &input, const CDataType &weights )
{
    if (input.empty())
        std::cout << "RoomOracle <startFeature> no candidate points input was empty" << std::endl;

    for (SoundData<CDataType>& elem : input)
    {
        elem.print(" Room Oracle feeding microphone and will print sounData ");
        m_array.feed(elem, weights);
    }
}

void roomOracle::startFeature()
{
    m_angleProb.clear();
    m_digger.resultVec.clear();
    auto sourceArc = m_digger.getSourceArc();
    for (auto& elem : *sourceArc)
    {
        feedTrainer( elem.data->begin(), elem.angle);
    }
    m_array.renewBuffers();
    fftWeight();
}


void roomOracle::feedTrainer(const DataConstIter data, int angle)
{
    m_featureOutput.setMethod(m_bestMethod[0]);
    m_featureOutput.process(data);
    m_trainer.Probability(0, m_featureOutput);
    m_featureOutput.clear();
    m_digger.pushResult(angle, m_trainer.getResult());
}

void
roomOracle::fftWeight()
{
    const int N = m_array.getElemCount() + 1;

    std::vector< std::complex<double> > in (N, 0.5);
    std::vector< std::complex<double> > out (N);
    //std::vector< std::complex<double> > polarOut (N);

    auto middleElem = in.begin() + in.size()/2;
    std::fill( middleElem - 2, middleElem + 3, 1); //zero phase

    auto plotIn = new sharpPlot("Angles", "Values", false);
    plotIn->drawBasicGraph(in, -45, 90 / N); plotIn->update();

    fftw_plan my_plan  = fftw_plan_dft_1d(N, reinterpret_cast<fftw_complex*>(&in[0]),
                                             reinterpret_cast<fftw_complex*>(&out[0]), FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(my_plan);

    auto plotOut = new sharpPlot("WeightsOut", "ApartureOut", false);
    plotOut->drawBasicGraph(out); plotOut->update();

    auto outMiddle = out.begin() + out.size()/2;
    m_weight.assign(outMiddle, out.end());
    m_weight.insert( m_weight.end(), out.begin() + 1, outMiddle);
    //m_weight.assign(out.begin() + 1 , out.end());

    auto plot = new sharpPlot("Weights", "Aparture", false);
    plot->drawBasicGraph(m_weight); plot->update();
//    std::transform( out.begin(), out.end(), polarOut.begin(),
//                    []( auto& in ){
//                                    return  std::complex<double>( std::abs(in), std::arg(in) );
//                                  } );

}


void roomOracle::parseValidation()
{
    std::ifstream methodOutput("speakerMethod.txt");
    std::string tempCurLine;
    while (std::getline(methodOutput, tempCurLine))
    {
        int speakerID = std::stoi(tempCurLine.substr(0, tempCurLine.find(",")));
        Method tempBestMethod = (Method)(std::stoi(tempCurLine.substr((tempCurLine.find(",") + 1))));
        m_bestMethod[speakerID] = tempBestMethod;
    }
}
