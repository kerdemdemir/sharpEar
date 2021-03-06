//Copyright (c) 2014,
//Kadir Erdem Demir
//All rights reserved.

//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//1. Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//2. Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//3. All advertising materials mentioning features or use of this software
//   must display the following acknowledgement:
//   This product includes software developed by the <organization>.
//4. Neither the name of the <organization> nor the
//   names of its contributors may be used to endorse or promote products
//   derived from this software without specific prior written permission.

//THIS SOFTWARE IS PROVIDED BY Kadir Erdem Demir ''AS IS'' AND ANY
//EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ROOMDIALOGS_H
#define ROOMDIALOGS_H

#include <QMessageBox>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <utility/utility.h>
#include <soundIO/IOManager.h>
#include <functional>
#include <QObject>
#include <environment/roomOracle.h>
#include <QVariant>
#include <QScriptValue>
#include <utility/multAccessData.h>

class microphoneNode;
class roomAtom;
class QGraphicsScene;
class roomSimulation;


class roomDialogs : public QObject
{
    Q_OBJECT

public:

    roomDialogs( const packetSound &sound, const roomVariables &room, microphoneNode& array):
        m_audioIO(sound, room), m_oracle(sound.samplesPerSec, sound.samplePerOutput, 0, 3, array)
    {
        m_soundParameters = sound;
        m_roomVariables = room;
        m_mouseClick = false;
        m_packetCount = 0;
        m_id = 0;
        m_NoiceAngle = -999;
        isLookForLocationOnly = false;
    }

    virtual ~roomDialogs()
    {

    }

    int process();
    int postprocess();
    void writeWav();
    int createDialog(roomAtom* ptrAtom);

    void
    sendAlertBox(const char* message)
    {
        QMessageBox msgBox;
        msgBox.setText(message);
        msgBox.exec();
        std::cout << "RoomDialogs <Alert> an alert send: " << message << std::endl;
    }


    bool isSourceExist()
    {
        for (auto& elem : m_cord2Data)
        {
            if (elem.second.isSource())
                return true;
        }
        return false;
    }

    bool isSoundExist()
    {
        for (auto& elem : m_cord2Data)
        {
            if (elem.second.isSound())
                return true;
        }
        return false;
    }


    bool mouseReleased(const QPointF& pos)
     {
        if ((m_mouseClick) && (pos == m_lastPoint))
         {
             return true;
         }
         return false;
     }

     void mouseClicked(const QPointF& pos)
     {
         m_lastPoint = pos;
         m_mouseClick = true;
     }

     void setFileName(const std::string& fileName)
     {
         m_fileName = fileName;
         std::cout << " Room Dialogs <Set fileName> A new fileName been set, fileName: " << m_fileName << std::endl;
     }


     void setRoomSimulation( roomSimulation* mainWindow )
     {
        m_oracle.setRoomSimulation(mainWindow);
     }

    roomOracle& getOracle()
    {
        return m_oracle;
    }

    void setRoomParams( const roomVariables& newParams )
    {
        m_roomVariables = newParams;
    }

public slots:

    int insertSound( roomAtom* atom, QString soundFileName, QString soundType )
    {
        m_fileName = soundFileName.toStdString();
        return openAudio(atom, str2SType(soundType.toStdString().c_str()));
    }

    void setWeight( QScriptValue realPart, QScriptValue imagePart )
    {
        CDataType weightList;
        QVariantList realPartVariantList = realPart.toVariant().toList();
        QVariantList imagePartVariantList = imagePart.toVariant().toList();
        for ( int i = 0; i < realPartVariantList.size(); i++  )
        {
            if ( imagePartVariantList.empty() )
                weightList.push_back(std::complex<double>( realPartVariantList[0].toDouble(), 0.0));
            else
                weightList.push_back(std::complex<double>( realPartVariantList[0].toDouble(), imagePartVariantList[0].toDouble()));
        }
        m_oracle.setWeights(weightList);
    }

    void setSpeakerTracking( bool in )
    {
        m_oracle.setSpeakerTracking(in);
    }

    void insertNull( int angle );
    void listen( roomAtom* atom );
    void setNoiceAngle( int angle )
    {
        m_NoiceAngle = angle;
    }

    void setLookAngle( int angle )
    {
        m_oracle.setLookAngle( angle );
    }

    void setForLocationOnly( bool in )
    {
        isLookForLocationOnly = in;
    }

    void clear()
    {
        m_cord2Listen.clear();
        m_cord2Data.clear();
        m_oracle.clear();
        m_audioIO.clear();
        m_packetCount = 0;
    }

private:

     bool m_mouseClick;
     std::map< Point, SoundData<CDataType> > m_cord2Data;
     std::map< Point, std::pair<int, roomAtom* > > m_cord2Listen;
     int m_id;
     int m_NoiceAngle;
     bool isLookForLocationOnly;
     size_t m_packetCount;
     std::string m_fileName;
     QPointF m_lastPoint;
     IOManager m_audioIO;
     roomOracle m_oracle;
     packetSound m_soundParameters;
     roomVariables m_roomVariables;

     int openAudio(roomAtom* atom, STypes soundType);
     bool isCordinateExist(Point scenePos);
};
#endif // ROOMDIALOGS_H
