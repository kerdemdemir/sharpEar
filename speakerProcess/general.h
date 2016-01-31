#ifndef GENERAL
#define GENERAL

#include <string>
#include <vector>
#include <range/v3/core.hpp>
#include <opencv2/core.hpp>
#include <array>
#include <memory>
#include <QFileInfo>
#include <QDir>

#define NUMBER_OF_PEOPLE 9
#define LOAD 0

using DataType2D = cv::Mat;

static constexpr size_t hopSize = 256;
static constexpr size_t win_s = 1024;
static constexpr size_t sampleRate = 16000;


template < typename T, typename U>
inline
void copyRangeToArray( T range, U& array )
{
    int pos = 0;
    RANGES_FOR( auto elem, range )
    {
        array[pos++] = elem;
    }
}

template < typename T, typename U>
inline
void copyComplexRangeToArray( T range, U& array )
{
    int pos = 0;
    RANGES_FOR( auto elem, range )
    {
        array[pos++] = elem.real();
    }
}

inline
size_t fileName2State(  std::string fileName )
{
    QFileInfo fileInfo = QString(fileName.c_str());
    if ( fileInfo.exists())
        fileName = fileInfo.fileName().toStdString();

    size_t state = 0;
    if (fileName[0] == 'm')
        state = 4;
    state += (fileName[1] - 49);
    return state;
}


inline
std::vector<std::pair< std::string, std::string> >
getFileNames (  const std::string& filePath )
{
    QDir dir(filePath.c_str());
    QStringList filters;
    filters << "*.wav";
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList();
    std::vector<std::pair< std::string, std::string> > fileNames;

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);

        if ( fileInfo.fileName().startsWith("cc") )
        {
            QString newPath = fileInfo.canonicalPath() + QDir::separator() + "m4" +   fileInfo.baseName() + "." + fileInfo.completeSuffix();
            rename( fileInfo.absoluteFilePath().toStdString().c_str(), newPath.toStdString().c_str());

        }


        fileNames.push_back( std::make_pair( fileInfo.absoluteFilePath().toStdString(), fileInfo.fileName().toStdString()));
    }

    return fileNames;
}

template< typename T >
inline
void getBestIndexAround ( T list, int& index, int offSet )
{
    double maxVal = 0;
    double bestIndex = 0;
    for ( int i = index - offSet; i < index + offSet; i++ )
    {
        if ( list[i] > maxVal )
        {
            bestIndex = i;
            maxVal = list[i];
        }
    }
}

inline
std::vector<std::string> splitString ( const std::string& line )
{
    std::vector<std::string> returnList;
    std::stringstream ss(line);

    std::string str;
    while(std::getline(ss, str, ',')) {
        returnList.push_back(str);
    }

    return returnList;
}

template < size_t N, typename U >
inline
std::array<size_t, N> sortIndexes(const U &v) {

  // initialize original index locations
  std::array<size_t, N> idx = {0};
  for (size_t i = 0; i != N; ++i) idx[i] = i;

  // sort indexes based on comparing values in v
  std::sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

  return idx;
}


#endif // GENERAL

