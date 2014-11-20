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

class interActionManager;
class QWidget;
class QGraphicsScene;
class roomAtom;
class microphoneNode;
class sharpPlot;
class roomDialogs;


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
    void startVisulution();
    void reset(valuesBasicUserDialog& userValues);
    //**SoundFile open write**//
    int openFile(std::string fileName);
    void setFocus(double focusDistance, std::vector<double> relativeSourceDist);
    void setFileName(const std::string& input);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    void calcRoomParameters();
    void assignSoundSource();
    void threadProcess();
    void startAtomColoring();
    void reset();
    void multiThreadImageCalc();
    roomAtom* findClosePoint(int xPos, int yPos);

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

    std::vector<roomAtom*> hndl2Atom;
    Worker _workerThread;

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

    void getAllNoicesArc(roomAtom *arcCenter);
};

#endif // ROOMSIMULATION_H