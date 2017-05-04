#ifndef SORTEDBESTPICKLIST
#define SORTEDBESTPICKLIST

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "environment/roomAtom.h"

class roomAtom;

template <typename T>
std::vector<size_t> sort_indexes(const std::vector<T> &v) {

  // initialize original index locations
  std::vector<size_t> idx(v.size());
  std::iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  std::sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}

struct SortedValue
{
    double val;
    double ratio;
    double index;

    double bestval;
    roomAtom* atom;

    bool operator > ( const SortedValue& rhs )
    {
        return val > rhs.val;
    }

    bool operator < ( const SortedValue& rhs )
    {
        return val < rhs.val;
    }

    double multipleAll()
    {
        return val*ratio*index;
    }

    void operator +=( const SortedValue& rhs )
    {
        bestval = std::max(bestval, rhs.bestval);
        val += rhs.val;
        ratio += rhs.ratio;
        index +=rhs.index;
    }

    std::string toString()
    {
        return std::to_string(val) + " ratio: " + std::to_string(ratio) + " index: " + std::to_string(index)
                + " atom real values radius: " + std::to_string(atom->getInfo().getRadius()) + " angle " + std::to_string(atom->getInfo().getAngle());
    }
};

class SortedBestPickList
{
public:
    using pairType = std::pair< double, SortedValue >;

    SortedBestPickList() = default;
    SortedBestPickList( size_t size, int offSet, bool isPrint )
    {
        maxSize = size;
        this->offSet = offSet;
        this->isPrint = isPrint;
    }
    SortedBestPickList( const SortedBestPickList& rhs )
    {
        this->maxSize = rhs.maxSize;
        this->offSet = rhs.offSet;
        this->isPrint = rhs.isPrint;
        this->pairList = rhs.pairList;
    }

    void insert( double in, double val, double ratio, double indexVal, roomAtom* atom )
    {
        auto newVal = SortedValue{val, ratio, indexVal,val, atom};
        auto pair = std::make_pair(in, newVal );
        auto index = findByOffSet(in);
        if ( index != -1)
        {
            if ( val > pairList[index].second.val )
                pairList[index] = pair;

            sortList();
            return;
        }

        if ( pairList.size() < maxSize )
        {
            pairList.push_back( pair );
        }
        else if ( pairList.back().second < newVal )
        {
            pairList.back() = pair;
        }

        sortList();

    }

    void print()
    {
        if ( isPrint == false )
            return;
        for ( auto elem : pairList)
        {
            std::cout <<  elem.first << " and its value " << elem.second.toString() << std::endl;
        }

    }

    std::vector<pairType> getPairList() const
    {
        return pairList;
    }

    void resize( int size )
    {
      pairList.resize(size);
    }

    void sortListByRatioIndex( int newSize )
    {
        std::sort( pairList.begin(), pairList.end(), []( pairType lhs, pairType rhs)
        {
            return (lhs.second.index * lhs.second.ratio)  > (rhs.second.index * rhs.second.ratio);
        });
        pairList.resize(newSize);
    }

    void sortListByIndex()
    {
        std::sort( pairList.begin(), pairList.end(), []( pairType lhs, pairType rhs)
        {
            return (lhs.second.index)  > (rhs.second.index);
        });
    }

    double getFirstRatio()
    {
        if (pairList.empty())
            return 0;
        return pairList.front().second.ratio;
    }

    double getFirstVal()
    {
        if (pairList.empty())
            return 0;
        return pairList.front().second.val;
    }

    double getFirstIndex()
    {
        if (pairList.empty())
            return 0;
        return pairList.front().second.index;
    }


    double getBestRealKeyValue()
    {
        return pairList.front().first;
    }

    std::vector<double> getBestRealKeyValuesInList()
    {
        std::vector<double>  returnVal;
        for ( size_t i= 0; i < pairList.size(); i++ )
        {
            returnVal.push_back( pairList[i].first);
        }
        return returnVal;
    }


private:

    int findByOffSet( double value )
    {
        for ( size_t i = 0; i < pairList.size(); i++)
        {
            if ( pairList[i].first - offSet <= value && pairList[i].first + offSet >= value )
                return i;
        }
        return -1;

    }

    void sortList()
    {
        std::sort( pairList.begin(), pairList.end(), []( pairType lhs, pairType rhs)
        {
            return lhs.second > rhs.second;
        });
    }


    bool isPrint;
    int offSet = 0;
    size_t maxSize = 0;
    std::vector< pairType > pairList;
};

class SortedBestPickListScorer
{
public:

    SortedBestPickListScorer( size_t maxSize, size_t binSize )
    {
        resultList.resize( maxSize / binSize );
        m_binSize = binSize;
    }

    void feed( const SortedBestPickList& in )
    {
        std::vector <std::pair< double, SortedValue >> list = in.getPairList();

        for ( size_t i = 0; i < list.size(); i++  )
        {
            resultList[list[i].first / m_binSize] += list[i].second.val * (list.size() -  i) ;
        }

    }

    int getBestRadius()
    {
        auto bestIter = std::max_element( resultList.begin(), resultList.end() );
        return std::distance( resultList.begin(), bestIter ) * m_binSize + m_binSize/2;
    }

    void clear()
    {
        std::fill(resultList.begin(), resultList.end(), 0.0);
    }

    std::vector<  double > resultList;
    size_t m_binSize;

};
#endif // SORTEDBESTPICKLIST

