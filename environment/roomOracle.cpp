#include <environment/roomOracle.h>


void roomOracle::preprocess(const std::vector<ref_t<SoundData<CDataType> > > &input )
{
   std::vector<double> weights(m_array.getElemCount(), 1);
   feedArray(input, weights);
}

void roomOracle::postprocess(const std::vector<ref_t<SoundData<CDataType> > > &input)
{
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

void roomOracle::startFeature( const std::map< int, std::vector<double> >& input )
{
    m_angleProb.clear();
    for (auto& elem : input)
    {
        feedTrainer(elem.second.begin(), elem.first);
    }
    m_array.renewBuffers();
    fftWeight();
}


void roomOracle::feedTrainer(const DataConstIter data, int angle)
{
    //m_featureOutput.setMethod(m_bestMethod[speakerID]);

    m_featureOutput.setMethod(m_bestMethod[0]);
    m_featureOutput.process(data);
    //m_featureOutput.Probability(speakerID, noiceID, featureOutput);
    m_trainer.Probability(0, 3, m_featureOutput);
    m_featureOutput.clear();
    m_angleProb.push_back( std::make_pair(angle, m_trainer.getResults() ));
}

void
roomOracle::fftWeight()
{
     using angle2Probility = std::pair<int, snrHelper>;
     double maxNoice = 0;
     double maxSound = 0;
     int noiceAngle = 0;
     int soundAngle = 0;

     std::sort(m_angleProb.begin(), m_angleProb.end(), [](const angle2Probility & a, const angle2Probility & b )
     {
            return a.first < b.first;
     } );
     for (size_t i = 0;i < m_angleProb.size(); i++)
     {
        if (i == 0)
            m_angleProb[i].second = ( m_angleProb[i].second  +  m_angleProb[i + 1].second  +  m_angleProb[i + 2].second ) / 3;
        else if (i < m_angleProb.size() - 1)
            m_angleProb[i].second  = ( m_angleProb[i - 1].second  + m_angleProb[i].second  + m_angleProb[i + 1].second ) / 3;
        else
             m_angleProb[i].second  = ( m_angleProb[i - 2].second  + m_angleProb[i - 1].second  + m_angleProb[i].second ) / 3;

     }

     for (auto elem : m_angleProb)
     {
         std::cout << " Room Oracle <fftWeight> Angle value " << elem.first << " Probibilty Speaker: " << elem.second.getFirst() <<
                   " Probibility Noice: " << elem.second.getSecond() << std::endl;
         if (elem.second.getSecond() > maxNoice)
         {
             maxNoice = elem.second.getSecond();
             noiceAngle = elem.first;
         }

         if (elem.second.getFirst() > maxSound)
         {
             maxSound = elem.second.getFirst();
             soundAngle = elem.first;
         }
     }

     soundAngle += 90;
     noiceAngle += 90;

     std::cout << " Room Oracle <fftWeight> Sound is at angle " << soundAngle << " Noice is at angle " << noiceAngle <<  " Now inverse fft will be taken" << std::endl;
     std::vector<double> data(m_array.getElemCount(), 1);
     m_weight.resize(m_array.getElemCount());
     data[noiceAngle / m_array.getElemCount()] = 0;

     fftw_plan tempPlan =  fftw_plan_r2r_1d(m_array.getElemCount(), data.data(), m_weight.data(), FFTW_HC2R, FFTW_ESTIMATE);
     fftw_execute(tempPlan);

     std::cout << "  Room Oracle <fftWeight> IFFT complete now will print values " << std::endl;

     for (auto elem : m_weight)
     {
         std::cout << elem << " ";
     }
     std::cout << std::endl;

     std::cout << "  Room Oracle <fftWeight> end" << std::endl;
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
