#ifndef MULTACCESSDATAMAP_H
#define MULTACCESSDATAMAP_H

#include <memory>
#include <unordered_map>
#include <unordered_map>
#include <algorithm>


template< typename T >
struct radAngData
{
    radAngData (const T& dat, int rad, int ang) : data(dat), radius(rad), angle(ang)
    {
    }

    T data;
    int radius;
    int angle;
};

template < typename T >
class radAngMultAccess
{
public :

    using radAngMap = std::unordered_map < int, std::vector< radAngData<T> > >;
    using radAngMapIte = typename radAngMap::iterator;

    void insert( int rad, int ang, const T& input )
    {
        auto radIte = radiusData.find(rad);
        if (radIte == radiusData.end())
            radiusData[rad].emplace_back(input, rad, ang);
        else
        {
            radIte->second.emplace_back(input, rad, ang);
        }

        auto angIte = angleData.find(ang);
        if (angIte == angleData.end())
            angleData[ang].emplace_back(input, rad, ang);
        else
        {
            angIte->second.emplace_back(input, rad, ang);
        }
    }

    void remove( int rad, int ang )
    {
        auto radIte = radiusData.find(rad);
        if (radIte != radiusData.end())
            radiusData.erase(radIte);

        auto angIte = angleData.find(ang);
        if (angIte != angleData.end())
            angleData.erase(angIte);
    }

    std::vector<radAngData<T>>* getByAngle( int angle )
    {
        auto ite = angleData.find(angle);
        if (ite == angleData.end())
            return nullptr;
        else
            return std::addressof(ite->second);
    }

    std::vector<radAngData<T>>* getByRadius( int radius )
    {
        auto ite = radiusData.find(radius);
        if (ite == radiusData.end())
            return nullptr;
        else
            return std::addressof(ite->second);
    }

    int findClosestRadius ( int radius )
    {
        int minDiff = INT_MAX;
        for ( auto& elem : radiusData )
        {
             auto diff = abs(elem.first - radius);
            if (diff < minDiff)
                minDiff = elem.first;
        }

        return minDiff;
    }

    std::vector<T> getAllData()
    {
        std::vector<T> returnVal;
        for (const auto& elemVec : radiusData)
        {
            for( const auto& elem : elemVec.second  )
            {
                returnVal.push_back(elem.data);
            }
        }
        return returnVal;
    }



    std::vector< radAngData<T> > getAllValue()
    {
        std::vector< radAngData<T> > returnVal;
        for (const auto& elemVec : radiusData)
        {
            for( const auto& elem : elemVec.second  )
            {
                returnVal.push_back(elem);
            }
        }
        return returnVal;
    }

    bool empty()
    {
         return radiusData.empty();
    }

protected:

    radAngMap radiusData;
    radAngMap angleData;
};


template< typename T >
struct radAngDataSummer : public radAngMultAccess < T >
{
    double sumAngle( std::vector< radAngData<T> >& in)
    {
        std::vector<double> tempSumMap;
        std::vector< radAngData<T> >& sharedVec = in;
        tempSumMap.resize(sharedVec.front().data->size(), 0);
        for (const auto& sharedData : sharedVec)
        {
            std::transform(sharedData.data->begin(), sharedData.data->end(), tempSumMap.begin(), tempSumMap.begin(),
                           [](double lhs, double rhs){
                return fabs(lhs) + fabs(rhs);
            });
        }

        return std::accumulate(tempSumMap.begin(), tempSumMap.end(), 0.0);
    }


    std::vector< std::pair < int, double > >
    findMaxOffSet (int angle, int offSet)
    {
        int angleStart = angle - offSet < -90 ? -90 : angle - offSet;
        int angleStop = angle + offSet > 90 ? 90 : angle + offSet;
        return findMax( angleStart, angleStop);
    }

    // Operand of the std::transform eg (std::plus) could be pass as Template or Functor this func could be more generic.
    std::vector< std::pair < int, double > >
    findMax (int angleStart, int angleStop)
    {
        std::vector< std::pair < int, double > > angleSum;
        for (auto& elem : this->angleData)
        {
            if ( elem.first > angleStart && elem.first < angleStop)
                continue;

            angleSum.push_back(std::make_pair(elem.first, sumAngle(elem.second)));
        }

        double totalEnergy = std::accumulate(angleSum.begin(), angleSum.end(), 0.0,
                [](double total, std::pair < int, double >&  val)
                {
                    return total + val.second;
                });


        std::transform(angleSum.begin(), angleSum.end(), angleSum.begin(),
        [totalEnergy]( std::pair < int, double >& lhs )
        {
            lhs.second /= totalEnergy;
            return lhs ;
        });

        std::sort(angleSum.begin(), angleSum.end(),
                  []( const std::pair < int, double >& lhs, const std::pair < int, double >& rhs)
        {
             return lhs.first < rhs.first;
        });

        //Sum previous and next elements to get bandwidth
        auto curElem = angleSum.begin();
        int counter = 0;
        std::vector< std::pair < int, double > > angleNewSum = angleSum;
        angleNewSum[counter].second = (curElem + 1)->second + 2*curElem->second;
        while ( curElem++ != angleSum.end() -1)
        {
            angleNewSum[++counter].second = curElem->second + ((curElem - 1)->second + (curElem + 1)->second);
        }
        angleNewSum[counter].second = (curElem - 1)->second + 2*curElem->second;


        std::sort(angleNewSum.begin(), angleNewSum.end(),
                  []( const std::pair < int, double >& lhs, const std::pair < int, double >& rhs)
        {
             return lhs.second > rhs.second;
        });

        return angleNewSum;
    }

};


#endif // MULTACCESSDATAMAP_H
