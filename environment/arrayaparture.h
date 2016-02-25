#ifndef ARRAYAPARTURE
#define ARRAYAPARTURE

#include <vector>
#include <utility/soundData.h>
#include <utility/commons.h>
#include <QPoint>
#include <unordered_map>

static constexpr int FILTERSIZE = 11;

enum class ArrayFocusMode
{
    NO_FOCUS,
    RADIUS_FOCUS,
    POINT_FOCUS
};

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
    }

    double
    getDelay( double focusDist, int steeringAngle, ArrayFocusMode mode ) const
    {
        if ( mode == ArrayFocusMode::NO_FOCUS)
            return 0;
        else if ( mode == ArrayFocusMode::RADIUS_FOCUS )
            return pow(m_distCenter, 2) / (2.0 * focusDist);

        double dist = m_distCenter * sin(steeringAngle * GLOBAL_PI / 180) +   pow(m_distCenter, 2) / (2.0 * focusDist);
        return dist / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec ;
    }

    double
    getDistDelay ( double focusDist  ) const
    {
        return focusDist / GLOBAL_SOUND_SPEED * (double)m_SoundParameters.samplesPerSec ;
    }

    void feed ( const SoundData<CDataType>& input  )
    {
      using leapIter =  std::unordered_map< int, std::shared_ptr<CDataType>  >::iterator;

      double delay = getDistDelay( input.getDistance(m_pos) ) - m_timeDelay;
      //double delay = getDelay(input.getInfo().getDistance(), input.getInfo().getAngle(),ArrayFocusMode::NO_FOCUS );
      CDataConstIter beginIter = input.getData();
      leapIter leapIte = m_leapData.find(input.getID());
      if (leapIte == m_leapData.end())
      {
          leapIte = m_leapData.emplace( input.getID(),
                                        std::make_shared<CDataType>( delay ) ).first;
      }
      auto tempLeap = *leapIte->second;



      for (size_t k = 0; k < m_apartureData.size() + delay - 1; k++)
      {
          if (k < delay )
          {
                m_apartureData[k] += tempLeap[k] * m_weight;
          }
          else if (k < m_apartureData.size())
          {
              SingleCDataType soundData = *beginIter++;
              m_apartureData[k] += soundData * m_weight;
          }
          else
          {
              SingleCDataType soundData = *beginIter++;
              leapIte->second->at(k - m_apartureData .size()) = soundData;
              m_sumLeapData[k - m_apartureData .size()] += soundData * m_weight;
          }
      }
    }

    void adjustArrayFocus( const SoundInfo& in, ArrayFocusMode mode )
    {
        m_timeDelay = getDelay( in.getRadius(), in.getAngle(), mode);
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

        m_timeDelay = 0;
    }

    const CDataType& getData() const
    {
        return m_apartureData;
    }

    std::complex<double> getData( size_t index ) const
    {
        if ( m_apartureData.size() > index )
            return m_apartureData[index + 1] ;
        else
        {
            auto leapIndex = index - m_apartureData.size() + 1;
            return m_sumLeapData[leapIndex];
        }
    }

    std::complex<double> getFocusData( size_t index ) const
    {
       return getData( index - m_timeDelay);
    }

private:

    double m_distCenter;
    QPoint m_pos;
    int  m_index;
    double  m_timeDelay;
    bool isCenter;
    std::complex<double> m_weight;

    std::unordered_map<int, std::shared_ptr<CDataType>>  m_leapData; // Leap Data for each source
    CDataType  m_sumLeapData; // Leap Data for each source
    CDataType  m_apartureData; // Leap Data for each source

    packetSound   m_SoundParameters;
    roomVariables m_RoomVariables;
};




#endif // ARRAYAPARTURE

