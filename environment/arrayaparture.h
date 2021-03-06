#ifndef ARRAYAPARTURE
#define ARRAYAPARTURE

#include <vector>
#include <utility/soundData.h>
#include <utility/commons.h>
#include <QPoint>
#include <unordered_map>
#include <utility/utility.h>

static constexpr int FILTERSIZE = 11;

enum class ArrayFocusMode
{
    POINT_FOCUS,
    RADIUS_FOCUS,
    STEER_FOCUS,
};

using leapMap =   std::unordered_map< int, std::shared_ptr<CDataType>  >;
using leapIter =  leapMap::iterator;


class ArrayAparture
{

public:
    ArrayAparture( const packetSound& soundConfig, const roomVariables& roomConfig, int index )
    {
        m_SoundParameters = soundConfig;
        m_RoomVariables = roomConfig;
        isCenter = false;
        m_index = index;

        m_sumLeapData.resize( soundConfig.samplePerOutput );
        m_apartureData.resize( soundConfig.samplePerOutput );
        maximumDelay = getMicMaxDelay();
    }

    int
    getDistDelay ( double focusDist  ) const
    {
        return focusDist / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
    }

    void feed ( const SoundData<CDataType>& input  )
    {
      size_t delay = getSteeringDelay( input.getAngle() ) + getFocusDelay( input.getRadius() ) + maximumDelay ;
      CDataConstIter beginIter = input.getData();
      leapIter leapIte = getLeapIter(input, delay);
      auto tempLeap = *leapIte->second;
      for (size_t k = 0; k < m_apartureData.size() + delay; k++)
      {
          if (k < delay )
          {
                m_apartureData[k] += tempLeap[k];
          }
          else if (k < m_apartureData.size())
          {
              SingleCDataType soundData = *beginIter++;
              m_apartureData[k] += soundData;
          }
          else
          {
              SingleCDataType soundData = *beginIter++;
              leapIte->second->at(k - m_apartureData .size()) = soundData;
              m_sumLeapData[k - m_apartureData .size()] += soundData;
          }
      }
    }

    leapIter getLeapIter( const SoundData<CDataType>& input, double delay )
    {
        leapIter leapIte = m_leapData.find(input.getID());
        if (leapIte == m_leapData.end())
        {
            leapIte = m_leapData.emplace( input.getID(),
                                          std::make_shared<CDataType>( delay ) ).first;
        }

        return leapIte;
    }

    void frequencyDomainWeighting( CDataType& in  )
    {
        in = sharpFFT(in, true);


        DataType temp;
        for ( auto& elem  : in)
            temp.push_back(std::abs(elem));

        auto maxIter =  std::max_element(temp.begin(), temp.end());

        auto dist = std::distance(temp.begin(), maxIter);
        (void)(dist);
        for ( int i = 100; i < 200; i++)
            in[i] *= m_weight;

        in = swapVectorWithIn(in);
        in = sharpFFT(in, false);

        for ( size_t i = 0; i < in.size(); i++ )
        {
            if ( i % 2 )
                in[i] /= -1.0;
            //std::conj(in[i]);
        }
    }

    void timeDomainWeighting( CDataType& in  )
    {
        for ( auto& elem : in )
        {
            elem *= m_weight;
        }
    }

    void filterSum( CDataType& frequencyResponse )
    {
        (void)(frequencyResponse);
        timeDomainWeighting(  m_apartureData );
        timeDomainWeighting(  m_sumLeapData );
    }

    void filterSum( )
    {
        frequencyDomainWeighting(  m_apartureData );
        frequencyDomainWeighting(  m_sumLeapData );
    }

    void filterSum( CDataType& frequencyResponse, CDataType& in )
    {
        in = sharpFFT(in, true);
        in = swapVectorWithIn(in);

        for ( size_t i = 0; i < in.size(); i++)
        {
            in[i] *= frequencyResponse[i].real();
        }

        in = sharpFFT(in, false);
    }

    int getSteeringDelay( double steeringAngle ) const
    {
        double returnVal = -m_distCenter * sin(steeringAngle * GLOBAL_PI / 180.0)  / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
        return std::floor(returnVal+0.5);
    }

    int getFocusDelay( double focusDist ) const
    {
        if ( focusDist < 0.1 )
            return 0;
        double returnVal =   (pow(m_distCenter, 2) / (2.0 * focusDist)) / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
        return std::floor(returnVal+0.5);
    }

    int getMicMaxDelay() const
    {
        double totalMicLen = ( m_RoomVariables.numberOfMics * m_RoomVariables.distancesBetweenMics -1 )/ 2.0;
        double returnVal = totalMicLen / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec;
        return std::floor(returnVal+0.5);
    }

    QPoint getPos() const
    {
        return m_pos;
    }

    double getDistCenter() const
    {
        return m_distCenter;
    }

    void setPos( const QPoint& pos )
    {
        m_pos = pos;
    }

    void setDistanceFromCenter( double dist )
    {
        m_distCenter = dist;
    }

    void setWeight( const std::complex<double>& in )
    {
       // m_weight = in * (double)(m_RoomVariables.numberOfMics);
        m_weight = in;
    }

    void clearData()
    {
        std::fill(m_sumLeapData.begin(), m_sumLeapData.end(), 0);
        std::fill(m_apartureData.begin(), m_apartureData.end(), 0);
    }

    void clearLeapData()
    {
        m_leapData.clear();
    }

    const CDataType& getData() const
    {
        return m_apartureData;
    }

    std::complex<double> getData( size_t index ) const
    {
        if ( m_apartureData.size() > index )
        {
            return m_apartureData[index];
        }
        else
        {
            auto leapIndex = index - m_apartureData.size();
            return m_sumLeapData[leapIndex];
        }
    }

private:

    double m_distCenter;
    QPoint m_pos;
    int  m_index;
    double  m_timeDelay;
    bool isCenter;
    std::complex<double> m_weight;
    size_t maximumDelay;
    std::unordered_map<int, std::shared_ptr<CDataType>>  m_leapData; // Leap Data for each source


    CDataType  m_sumLeapData; // Leap Data for each source
    CDataType  m_apartureData; // Leap Data for each source

    packetSound   m_SoundParameters;
    roomVariables m_RoomVariables;
};




#endif // ARRAYAPARTURE

