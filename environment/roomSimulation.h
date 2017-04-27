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

#ifndef ROOMSIMULATION_H
#define ROOMSIMULATION_H



#include <QGraphicsView>
#include <vector>
#include <utility/commons.h>
#include <utility/workerThread.h>
#include <utility/soundData.h>
#include "utility/multAccessData.h"
#include <QScriptValue>

class interActionManager;
class QWidget;
class QGraphicsScene;
class roomAtom;
class AtomList;
class microphoneNode;
class sharpPlot;
class roomDialogs;
class QScriptValue;
class QScriptContext;
class QScriptEngine;
//Will simulate the room with help of atoms and mic item
//Most probably will have responsibility of opening the sound also

class roomSimulation : public QGraphicsView
{
    Q_OBJECT

public:

    //** UI related functions **//
    roomSimulation(QRectF boudingRect, QWidget *parent = 0);
    void setRoomRect(QRectF roomRect);
    void setAtoms();
    void setRadiusAngleAtom();
    void defaultMicPos();
    void setBoundingLines();
    CDataType getImpulseResponce( CDataType& weights );
    void reset(valuesBasicUserDialog& userValues);
    //**SoundFile open write**//
    int openFile(std::string fileName);
    void setFocus(double focusDistance, std::vector<double> relativeSourceDist);
    void setFileName(const std::string& input);

    std::vector< roomAtom*> getMiddleAtoms();
    std::vector< roomAtom* > getAtomInRadius( int curRadius, bool isUnique = true, int start = -90, int offSet = 180 );
    std::vector< roomAtom* > getAtomsInAngle( int angle, int jump = 0, bool isUnique = false );
    std::vector<roomAtom*> hndl2Atom;

    roomDialogs* getDialog()
    {
        return _roomDialogs;
    }
    radAngMultAccess<roomAtom *> getArcRadius(roomAtom *arcCenter);
    bool isFirst = true;
    roomAtom* findAtomPolarImpl( double radius, double angle );
    double getRoomLen();
    roomAtom *findAtomPolarFromDataBase(double radius, double angle);
public slots:

    void startBeamforming();
    void reset();
    void reset( int microphoneNumber );
    void reset( int microphoneNumber, int distance );
    void resetAtoms(const roomVariables &_roomParameters);

    void listen(double radius, double angle);
    void insertSound( double radius, double angle, QString soundFileName, QString soundType );
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    void calcRoomParameters();
    void assignSoundSource();


    void multiThreadImageCalc();
    roomAtom* findClosePoint(int xPos, int yPos);
    Worker _workerThread;
    void threadProcess();
    void startAtomColoring();

    interActionManager* hndl_interActionManager;

    QRectF         _roomBoudingRect;
    roomAtom *     _maxAtomPointer;
    roomAtom *     _minAtomPointer;
    QGraphicsPixmapItem* _pixelImageItem;
    QGraphicsScene *_room_scene;
    QGraphicsView  *_room_QView;
    microphoneNode *_micArr;

    roomVariables _roomParameters;
    packetSound   _soundParameters;
    std::shared_ptr<AtomList>    atomDatabase;


    void drawAngleValueGraph();
    void drawYAxisGraph(int xAxisCordinates, bool lowPassFilter );
    void drawMiddleAxisGraph();
    void setArc();


    roomDialogs* _roomDialogs;

    sharpPlot* _sharpPlot1;
    sharpPlot* _sharpPlot2;

    double _focusDistance;
    std::pair< double, double > _relativeSourceDist;
    std::vector<std::pair< double, double > > _relativeSourceDistVec;

};

#endif // ROOMSIMULATION_H
