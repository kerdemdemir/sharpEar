#include "utility/utility.h"
#include <random>
#include "iostream"
#include "fstream"

std::ofstream outputFile;

bool operator <(QPoint point1, QPoint point2)
{
    if (point1.x() < point2.x())
        return true;
    else if (point1.x() > point2.x())
        return false;
    else
    {
        if (point1.y() <= point2.y())
            return true;
        else
            return false;
    }
    return true;
}




void
openFileIO()
{
    outputFile.open("logging.txt", std::ofstream::out);
    if (outputFile.is_open())
    {
        std::cout << "file is ok " << std::endl;
    }
    //if (ofile)
}

void
closeFile()
{
    outputFile.flush();
    outputFile.close();
}

void
writeToFile(const std::vector<double> &refVector)
{
    outputFile << " starting fileIO: ";
    for (auto& elem : refVector)
        outputFile << elem  << " ";
    outputFile << " finished fileIO ";
    outputFile << std::endl;
}

void
writeToFileWithIte(const std::vector<double>::iterator iteVect, size_t totalSize)
{
    outputFile << " starting fileIO raw: ";
    for (size_t i = 0; (i < totalSize); i++)
    {
        outputFile << *(iteVect + i)  << " ";
    }

    outputFile << " finished fileIO raw";
    outputFile << std::endl;
}

static  std::uniform_real_distribution<> dist(0, 360);
static std::random_device rd;
static std::mt19937 e2(rd());
double
randomGenerator()
{
    return dist(e2);
}

void writeToFile(const char* value)
{
    if (!outputFile.is_open())
        openFileIO();

    outputFile <<  value << std::endl ;
}
