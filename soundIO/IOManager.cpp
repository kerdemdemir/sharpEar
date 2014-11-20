#include <soundIO/IOManager.h>
#include <utility/soundData.h>

std::vector<double> IOParams::bufferData;

int
IOManager::init(std::string& fileName, int id, bool isPulse)
{
    if ( !isPulse && fileName.empty())
    {
        std::cout << "Please enter file name" << std::endl;
        return -1;
    }

    IOParams newIO(m_SoundParameters.samplePerOutput);
    if ( !isPulse && newIO.open(fileName, true) == -1 )
    {
        return -1;
    }
    m_ID2Params.emplace(std::make_pair(id, std::move(newIO)));
    std::cout << " audioIO:: <init> A new file initiliazed ID: " << id << std::endl;
    return 0;
}

int
IOManager::read(SoundData<CDataType> &output)
{
    ioIterator iter  = m_ID2Params.find(output.getID());
    if (iter == m_ID2Params.end())
    {
        std::cout << " audioIO:: <read> A sources cordinates existed but ID did not ID:"  << output.getID() << std::endl;
        return -1;
    }

    int readCount = !output.isPulse() ? iter->second.read() : iter->second.pulse(m_SoundParameters.samplesPerSec);
    if (readCount < 0)
        return -1;

    output.setStatus(iter->second.getReadStatus());
    std::vector<std::complex<double>>::iterator begin = iter->second.getDataIter();
    std::vector<std::complex<double>>::iterator end = iter->second.getDataEnd();
    output.setData(begin, end);
    std::cout << " audioIO:: <read> Data Read from ID: " << output.getID() << " Count: " << readCount << std::endl;
    return readCount;
}

int
IOManager::write(const std::vector<double> &data, int id,  const std::string &fileName)
{
    ioIterator iter  = m_ID2Params.find(id);
    if (iter == m_ID2Params.end())
    {
        std::cout << " audioIO:: <write> ID: "<< id << " didn't existed in cordinates will be created "  << std::endl;
        IOParams newIO(m_SoundParameters.samplePerOutput);
        newIO.setInfo( m_ID2Params.begin()->second.getInfo());
        m_ID2Params.emplace(std::make_pair(id, std::move(newIO)));
        iter = m_ID2Params.find(id);
    }

    if (!iter->second.isWritten())
        iter->second.open(fileName, false);// Don't forget about fail check

    return iter->second.write(data);
}

int IOManager::remove(int id)
{
    ioIterator iter  = m_ID2Params.find(id);
    if (iter == m_ID2Params.end())
    {
        std::cout << " audioIO:: <remove> A sources cordinates existed but ID did not ID:"  << id << std::endl;
        return -1;
    }
    return m_ID2Params.erase(id);
}
