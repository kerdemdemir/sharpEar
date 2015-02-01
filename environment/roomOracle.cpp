#include <environment/roomOracle.h>


void roomOracle::preprocess(const std::vector<ref_t<SoundData<CDataType> > > &input )
{
   std::vector<double> weights(m_array.getElemCount(), 1);
   feedArray(input, weights);
}

void roomOracle::postprocess(const std::vector<ref_t<SoundData<CDataType> > > &input)
{
    m_weight.resize(m_array.getElemCount(), 1);
    feedArray(input, m_weight);
}

void roomOracle::feedArray(const std::vector<ref_t<SoundData<CDataType> > > &input, const std::vector<double>& weights )
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
    std::vector<double> zerosOnesVec = m_digger.startDigging();
    std::vector<double> fftwData ( zerosOnesVec.size(), 0 );
    fftw_plan fftwPlan = fftw_plan_r2r_1d(zerosOnesVec.size(), zerosOnesVec.data(), fftwData.data(), FFTW_HC2R , FFTW_ESTIMATE);
    fftw_execute(fftwPlan);
    m_weight.resize(m_array.getElemCount(), 0);

    if (fftwData.size() > m_weight.size())
    {
        int extra = fftwData.size() - m_weight.size();
        int counter = 0;
        for (int i = extra/2; i < fftwData.size() - extra/2; i++, counter++)
        {
            if (counter > m_weight.size())
                break;
            m_weight[counter] = fftwData[i];
        }
    }
    else
    {
        int extra = m_weight.size() - fftwData.size() ;
        int counter = 0;
        for (int i = extra/2; i < m_weight.size() - extra/2; i++, counter++)
        {
            if (counter > fftwData.size())
                break;
            m_weight[i] = fftwData[counter];
        }
    }
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
