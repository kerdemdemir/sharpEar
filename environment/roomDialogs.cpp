#include "roomdialogs.h"
#include <iostream>
#include <QMessageBox>
#include <environment/microphoneNode.h>
#include <environment/roomAtom.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <soundIO/IOManager.h>
#include <audiperiph/trainer.h>
#include <memory>

void roomDialogs::writeWav()
{
    std::cout << " RoomDialogs <writeWav>: One iteration has finished will check for output" << std::endl;
    if( m_cord2Listen.empty())
    {
        std::cout << " RoomDialogs <writeWav>: Post process never happened won't write output" << std::endl;
    }
    else
    {
        std::vector<double> wholeData(m_soundParameters.samplePerOutput);
        for (auto elem : m_cord2Listen)
        {
            elem.second.second->sumWhole(wholeData);
            std::string fileName = std::to_string(elem.first.first); fileName += "_";
            fileName += std::to_string(elem.first.second); fileName += "_" ;
            fileName += STypes2Str(elem.second.second->getInfo().getType());
            fileName += ".wav";
            m_audioIO.write( wholeData, elem.second.first, fileName);
        }
    }
}

int
roomDialogs::postprocess()
{
    if (m_noicePoints.empty())
        return -1;

    std::cout << " RoomDialogs <process>: There are noices over the radius sound will make further process" << std::endl;
    std::vector< radAngData< roomAtom* > > allData = std::move(m_noicePoints.getAllValue());
    for (size_t i = 0; i < allData.size(); i++)
    {
        auto dataVal = std::move(allData[i].data->sumWhole());
        m_oracle.insertData(allData[i].data->getInfo(), allData[i].radius, allData[i].angle, dataVal) ;
    }
    m_oracle.startFeature();
    return 1;
}

int
roomDialogs::process()
{
    std::vector< ref_t< SoundData<CDataType> > > soundDataVec;
    if (!isSoundExist())
    {
        sendAlertBox("RoomDialogs <process>: No sound selected please select sound first");
        return -1;
    }
    std::cout << "roomDialogs <process>: A new process started process count: " << m_packetCount << std::endl;
    for (auto& elem : m_cord2Data)
    {
        if (!elem.second.isSound())
            continue;
        if ( m_audioIO.read(elem.second) < 0 )
        {
            sendAlertBox("RoomDialogs <process>: Read failed please be sure file is not deleted");
            return -1;
        }

        if (elem.second.getStatus() == SStatus::FINISHED)
        {
            std::cout << "RoomDialogs <process>: One of the sound files come to end will delete it" << std::endl;
            m_audioIO.remove(elem.second.getID());
            if (elem.second.isSource())
            {
                std::cout << " RoomDialogs <process>: Source File come to end was source file exit simulation" << std::endl;
                m_cord2Data.erase(elem.first);
                return -1;
            }
            m_cord2Data.erase(elem.first);
        }

        soundDataVec.push_back(elem.second);
    }

    std::cout << " RoomDialogs <process>: Room dialogs readData now will trigger Overseer" << std::endl;

    m_oracle.preprocess(soundDataVec);
    if (postprocess() != -1)
    {
        m_oracle.postprocess(soundDataVec);
    }
    writeWav();
    m_packetCount++;
    return 0;
}

int
roomDialogs::openAudio(roomAtom *curAtom, STypes soundType)
{
    Point curPosCM = curAtom->getInfo().getRealPos();
    if ( isCordinateExist(curPosCM))
    {
        sendAlertBox("roomDialogs <openAudio> A source already exists won't add new one");
        return -1;
    }

    if ( m_audioIO.init(m_fileName, m_id, (soundType == STypes::PULSE)) == -1)
    {
        sendAlertBox("roomDialogs <openAudio> Sound File is not valid enter filename correctly");
        return -1;
    }
    curAtom->getInfo().setType(soundType);
    curAtom->print();
    SoundData<CDataType> newSound( m_id, curAtom->getInfo());

    newSound.setStatus(SStatus::JUST_INITED);
    m_cord2Data[curPosCM] = std::move(newSound);
    std::cout << "roomDialogs <openAudio> A new file pushed its cordinates: " <<  curPosCM.first << "," << curPosCM.second
              << "Its ID: " << m_id++ << " Filename: " << m_fileName <<  std::endl;
    return 1;
}

bool
roomDialogs::isCordinateExist( Point scenePos )
{
    auto iter = m_cord2Data.find(scenePos);
    if (iter == m_cord2Data.end())
    {
        return false;
    }
    return true;
}

int roomDialogs::createDialog(roomAtom *ptrAtom)
{
    QMessageBox msgBox;
    msgBox.setText("You are adding a voice source.");
    msgBox.setInformativeText("Please describe if it is Source, Noise or you want to get output Listen?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore
                          | QMessageBox::Close | QMessageBox::Discard | QMessageBox::Apply);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setButtonText(QMessageBox::Ok, QString("Source sound"));
    msgBox.setButtonText(QMessageBox::Yes, QString("Gauss pulse"));
    msgBox.setButtonText(QMessageBox::No, QString("Output in this point"));
    msgBox.setButtonText(QMessageBox::Ignore, QString("Noice sound"));
    msgBox.setButtonText(QMessageBox::Discard, QString("Moving sound"));
    msgBox.setButtonText(QMessageBox::Apply, QString("Arc Output"));
    int ret = msgBox.exec();

    switch (ret) {
        case QMessageBox::Ok:
            std::cout << "A source sound is added " << std::endl;
            if (isSourceExist())
            {
              std::cout << "Source all ready exist add noice " << std::endl;
              sendAlertBox("Source all ready exist add noice ");
              return -1;
            }
            return openAudio( ptrAtom, STypes::SOURCE );
        case QMessageBox::Yes:
            std::cout << "A gauss pulse is added ";
            return openAudio( ptrAtom,  STypes::PULSE );
        case QMessageBox::No:
            m_cord2Listen[ptrAtom->getInfo().getRealPos()] = std::make_pair( m_id, ptrAtom );
            ptrAtom->getInfo().setOutput(true);
            ptrAtom->print();
            std::cout << "A listen point is added id = " <<  m_id++ << std::endl;
            return 0;
        case QMessageBox::Ignore:
            std::cout << "A noice sound is added ";
            return openAudio( ptrAtom,  STypes::NOICE );
        case QMessageBox::Discard:
            std::cout << "A moving sound is added ";
            if (isSourceExist())
            {
                std::cout << "Source all ready exist add noice " << std::endl;
                sendAlertBox("Source all ready exist add noice ");
                return -1;
            }
            return openAudio(ptrAtom,  STypes::MOVING_SOUND);
        case QMessageBox::Apply:
        {
            return openAudio(ptrAtom, STypes::SOURCE_NOICE);
        }
        case QMessageBox::Close:
        case QMessageBox::Cancel:
            return -1;
        default:
            return 0;
    }
}

void roomDialogs::setSoundNoices(const radAngMultAccess< roomAtom* >&noices)
{
   m_isPostProcess = true;
   m_noicePoints = noices;
   std::vector< roomAtom* > allData = std::move(m_noicePoints.getAllData());
   //const auto& allData = noices.getAllData();
   for (const auto& elem : allData)
   {
     m_cord2Listen[elem->getInfo().getRealPos()] = std::make_pair(m_id++, elem);
   }
}
