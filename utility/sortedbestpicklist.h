#ifndef SORTEDBESTPICKLIST
#define SORTEDBESTPICKLIST

#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

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
        return std::to_string(val) + " ratio: " + std::to_string(ratio) + " index: " + std::to_string(index);
    }
};

class SortedBestPickList
{
public:
    using pairType = std::pair< int, SortedValue >;

    SortedBestPickList( size_t size, int offSet, bool isPrint )
    {
        maxSize = size;
        this->offSet = offSet;
        this->isPrint = isPrint;
    }

    void insert( int in, double val, double ratio, double indexVal )
    {
        auto newVal = SortedValue{val, ratio, indexVal,val};
        auto pair = std::make_pair(in, newVal );
        auto index = findByOffSet(in);
        if ( index != -1)
        {
            if ( val > pairList[index].second.bestval )
                pairList[index].first = in;
            pairList[index].second += newVal;

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


    void sortListByIndex( int newSize )
    {
        pairList.resize(newSize);
        std::sort( pairList.begin(), pairList.end(), []( pairType lhs, pairType rhs)
        {
            return (lhs.second.index * lhs.second.ratio)  > (rhs.second.index * rhs.second.ratio);
        });
    }

    double getBestRealKeyValue()
    {
        return pairList.front().first;
    }

private:

    int findByOffSet( int value )
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
        std::vector <std::pair< int, SortedValue >> list = in.getPairList();

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

