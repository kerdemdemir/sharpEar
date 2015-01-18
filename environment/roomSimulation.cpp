#include "roomsimulation.h"
#include "interactor.h"
#include "iostream"
#include "QMessageBox"
#include "chrono"
#include "functional"
#include <QWidget>
#include <memory>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <environment/microphoneNode.h>
#include <environment/roomdialogs.h>
#include "roomAtom.h"
#include "sharpplot.h"
#include <QPen>
#include <utility/multAccessData.h>


//#define DEBUG_TEST_MODE
//#define POINT_ALWAYS_CENTER


roomSimulation::roomSimulation(QRectF boudingRect, QWidget *parent) :
    QGraphicsView(parent)
{
    hndl_interActionManager = interActionManager::getDataShareInstance();
    _focusDistance = 0;
    _room_scene = new QGraphicsScene(this);
    _room_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    _pixelImageItem = NULL;
    _room_scene->setSceneRect(boudingRect);
    setScene(_room_scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(FullViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(1, 1);
    setWindowTitle(tr("Room simulation"));
    calcRoomParameters();
    _micArr  = new microphoneNode(_soundParameters, _roomParameters);
    _roomDialogs = new roomDialogs(_soundParameters, _roomParameters, *_micArr);
    _room_scene->addItem(_micArr);
    setBoundingLines();
    defaultMicPos();
    if (ENABLE_RADIAN_ANGLE)
    {
        setRadiusAngleAtom();
    }
    else
        setAtoms();

    _sharpPlot1 = NULL;
    _sharpPlot2 = NULL;
}

void roomSimulation::setBoundingLines()
{
    QPen lineDrawPen = QPen(Qt::red);

    QLineF topLine(_room_scene->sceneRect().topLeft(), _room_scene->sceneRect().topRight());
    QLineF leftLine(_room_scene->sceneRect().topLeft(), _room_scene->sceneRect().bottomLeft());
    QLineF rightLine(_room_scene->sceneRect().topRight(), _room_scene->sceneRect().bottomRight());
    QLineF bottomLine(_room_scene->sceneRect().bottomLeft(), _room_scene->sceneRect().bottomRight());

    _room_scene->addLine(topLine, lineDrawPen);
    _room_scene->addLine(leftLine, lineDrawPen);
    _room_scene->addLine(rightLine, lineDrawPen);
    _room_scene->addLine(bottomLine, lineDrawPen);

}

//Dont forget to have default values
void roomSimulation::setRoomRect(QRectF roomRect)
{
    this->setSceneRect(roomRect);
}

//Needs to be implemented more flexible
void roomSimulation::defaultMicPos()
{
    QPoint sceneCenterQ( _room_scene->sceneRect().center().x(), _room_scene->sceneRect().top());
    Point sceneCenter = std::make_pair(_room_scene->sceneRect().center().x(), _room_scene->sceneRect().top());
    Point centerCM = std::make_pair((double)hndl_interActionManager->getBasicUserDialogValues()->listenRange/2, 0);
    _micArr->setPos(sceneCenterQ);
    _micArr->setNewPos(sceneCenter, centerCM);
}

void
roomSimulation::calcRoomParameters()
{

    _soundParameters.samplesPerSec = 16000; //* UP_SAMPLE_RATE;
     if (ENABLE_UPSAMPLING)
        _soundParameters.samplesPerSec *= UP_SAMPLE_RATE;

    _soundParameters.currentOutputTime = 20000.0 / _soundParameters.samplesPerSec;
    _soundParameters.amplitude = 0;
    _soundParameters.samplePerOutput = _soundParameters.currentOutputTime * _soundParameters.samplesPerSec;

    _roomParameters.numberOfMics = hndl_interActionManager->getBasicUserDialogValues()->micNumber;
    _roomParameters.distancesBetweenMics = hndl_interActionManager->getBasicUserDialogValues()->distanceBetweenMic;
    _roomParameters.yPixelCount = _room_scene->sceneRect().right() - _room_scene->sceneRect().left();
    _roomParameters.pixel2RealRatio = (double)hndl_interActionManager->getBasicUserDialogValues()->listenRange
                                        / (double)_roomParameters.yPixelCount;

    _roomParameters.pixel4EachAtom = 3;
    _roomParameters.angleDist = 2;
    _roomParameters.numberOfAtomsIn1D = (double)hndl_interActionManager->getBasicUserDialogValues()->listenRange
                                        / hndl_interActionManager->getBasicUserDialogValues()->dx_dy;

    _soundParameters.sumSize = (((_roomParameters.pixel4EachAtom *
        _roomParameters.pixel2RealRatio) / GLOBAL_SOUND_SPEED ) * _soundParameters.samplesPerSec);


    _roomParameters.maximumDelay = ((double)hndl_interActionManager->getBasicUserDialogValues()->listenRange / GLOBAL_SOUND_SPEED) * sqrt(2.0) * _soundParameters.samplesPerSec;
    //_roomParameters.maximumDelay  = _roomParameters.distancesBetweenMics * (_roomParameters.numberOfMics - 1)/ GLOBAL_SOUND_SPEED * _soundParameters.samplesPerSec;
    if (_soundParameters.sumSize >  40)
         _soundParameters.sumSize = 40;
    else if (_soundParameters.sumSize < 30)
        _soundParameters.sumSize = 30;
    _roomParameters.angleOfTheMicrophone = 0;

}

void
roomSimulation::reset(valuesBasicUserDialog& userValues)
{
    auto temp_point = _micArr->mapToParent(_micArr->boundingRect().center());
    _micArr->setDistCount(userValues.micNumber,  userValues.distanceBetweenMic);


    _micArr->setPos(temp_point.x() - _micArr->boundingRect().width()/2,
                    temp_point.y() - _micArr->boundingRect().height()/2);
    _roomParameters.numberOfMics = userValues.micNumber;
    _roomParameters.distancesBetweenMics = userValues.distanceBetweenMic;
}

void
roomSimulation::setAtoms()
{
    for (int x = _room_scene->sceneRect().left(); x < _room_scene->sceneRect().right()
                                                          ;x += _roomParameters.pixel4EachAtom)
    {
         for (int y = _room_scene->sceneRect().top(); y < _room_scene->sceneRect().bottom()
                                                          ;y += _roomParameters.pixel4EachAtom)
         {
            roomAtom* tempAtom = new roomAtom(_soundParameters, _roomParameters, *_micArr);
            _room_scene->addItem(tempAtom);            
            tempAtom->setPos(x, y);
            Point atomScenePos = std::make_pair(x, y);
            Point atomCMPos = std::make_pair((x - _room_scene->sceneRect().left()) * _roomParameters.pixel2RealRatio,
                                             (y - _room_scene->sceneRect().top()) * _roomParameters.pixel2RealRatio );
            tempAtom->createInfo( atomCMPos, atomScenePos );
            hndl2Atom.push_back(tempAtom);

         }
    }
}

void
roomSimulation::setRadiusAngleAtom()
{
    QPointF startPoint(_room_scene->sceneRect().center().x() ,
                       _room_scene->sceneRect().top());

    for (int angle = -90; angle <= 90; angle += _roomParameters.angleDist)
    {
        QPointF curPoint = startPoint;
        while (true)
        {
            curPoint.setX(curPoint.x() + _roomParameters.pixel4EachAtom * sin(angle * GLOBAL_PI / 180));
            curPoint.setY(curPoint.y() + _roomParameters.pixel4EachAtom * cos(angle * GLOBAL_PI / 180));
            if (!scene()->sceneRect().contains(curPoint))
                break;
            if (_room_scene->itemAt(curPoint, QTransform()) != NULL)
                continue;

            roomAtom* tempAtom = new roomAtom(_soundParameters, _roomParameters, *_micArr);
            _room_scene->addItem(tempAtom);
            tempAtom->setPos(curPoint.x(), curPoint.y());
            Point atomScenePos = std::make_pair(curPoint.x(), curPoint.y());
            Point atomCMPos = std::make_pair((curPoint.x() - _room_scene->sceneRect().left()) * _roomParameters.pixel2RealRatio,
                                             (curPoint.y() - _room_scene->sceneRect().top()) * _roomParameters.pixel2RealRatio );
            double angle = atan ( atomCMPos.first / atomCMPos.second ) * 180 / M_PI ;
            double radius = sqrt(pow(atomCMPos.first, 2) + pow(atomCMPos.second, 2));
            SoundInfo newInfo(atomScenePos, atomCMPos, angle, radius);
            tempAtom->getInfo() = newInfo;
            hndl2Atom.push_back(tempAtom);
        }
    }
}

void
roomSimulation::startVisulution()
{
    while (true)
    {
        if (_roomDialogs->process() == -1)
            return;
        threadProcess();
        startAtomColoring();
        _micArr->renewBuffers();
    }
}

void
roomSimulation::threadProcess()
{
    _workerThread.clearTasks();
    for (size_t i = 0; i < hndl2Atom.size(); i++)
    {
        roomAtom *atomPointer = hndl2Atom[i];
        std::shared_ptr< std::packaged_task< void() > > tempTask (new std::packaged_task<void()>(std::bind(
            &roomAtom::start, atomPointer)));
        _workerThread.post(tempTask);
    }
    _workerThread.process();
}

void
roomSimulation::startAtomColoring()
{
    double max = 0;
    double min = INT_MAX;
    for (size_t i = 0; i < hndl2Atom.size(); i++)
    {
        roomAtom *atomPointer = hndl2Atom[i];
        double temp = atomPointer->getResult().second;
        if (temp > max)
        {
            max = temp;
            _maxAtomPointer = atomPointer;
        }
        if (temp < min)
        {
            min = temp;
            _minAtomPointer = atomPointer;
        }
    }

    for (size_t i = 0; i < hndl2Atom.size(); i++)
    {

         roomAtom *atomPointer = hndl2Atom[i];
         atomPointer->setColor(true, min, max);
         atomPointer->update();

    }

    #ifdef DEBUG_TEST_MODE
        drawMiddleAxisGraph();
        drawAngleValueGraph();
    #endif

    this->scene()->update();
    this->viewport()->repaint();
}

roomAtom*
roomSimulation::findClosePoint(int xPos, int yPos)
{
    roomAtom *atom = qgraphicsitem_cast<roomAtom*>(_room_scene->itemAt( xPos, yPos, QTransform()));
    if (atom == NULL)
    {
        std::cout << "roomSimulation <findClosePoint> The position of the sound is not valid will search manually" << std::endl;
        double closestElement = INT_MAX;
        for (auto& elem : hndl2Atom)
        {
            auto tempDist = elem->getDistance(QPointF(xPos + _room_scene->sceneRect().topLeft().x(), yPos + _room_scene->sceneRect().top()), false);
            if (tempDist < closestElement)
            {
                closestElement = tempDist;
                atom =  elem;
            }
        }
        return atom;
    }
    return atom;
}

void
roomSimulation::getAllNoicesArc(roomAtom* arcCenter)
{
    radAngMultAccess< roomAtom* > noicePoints;
    //std::vector< roomAtom* > noicePoints;
    SoundInfo soundInfo = arcCenter->getInfo();
    Point centerScenePos = _micArr->getSceneMiddlePos();
    std::cout << " Room Simulation <getAllNoicesArc> A noiced sound is add so candidate noice points will be set " << std::endl;
    double angle = soundInfo.getAngle() ;
    double radius = soundInfo.getRadius();
    int radiusEpsilon = _roomParameters.pixel4EachAtom * _roomParameters.pixel2RealRatio;
    int radiusElemCount = 100 / radiusEpsilon; // 100cm is 1 meter

    for (int curRadius = (radius - radiusElemCount/2 * radiusEpsilon);
         curRadius < (radius + radiusElemCount/2 * radiusEpsilon);
         curRadius += radiusEpsilon)
    {
        for (int curAngle = angle - 45 ; curAngle < angle + 45; curAngle += 3)
        {
            double cosVal  = curRadius * cos(curAngle * GLOBAL_PI / 180.0);
            double yPos = _room_scene->sceneRect().top() + (abs(cosVal) / _roomParameters.pixel2RealRatio);
            double sinVal  = curRadius * sin(curAngle * GLOBAL_PI / 180.0);
            double xPos = centerScenePos.first + ( sinVal / _roomParameters.pixel2RealRatio );
            roomAtom *atom = findClosePoint(xPos, yPos);
            if (atom == NULL)
                continue;

            std::cout <<  "Room Simulation <getAllNoicesArc> A new output point added for Arc point will be printed" << std::endl;
            atom->getInfo().setOutput(true);
            atom->print();
            noicePoints.insert(curRadius, curAngle, atom);
        }
        std::cout <<  "Room Simulation <getAllNoicesArc> Radius " << curRadius << " done. " << std::endl;
    }
    _roomDialogs->setSoundNoices(noicePoints);
}

void
roomSimulation::setFileName(const std::string& input)
{
    _roomDialogs->setFileName(input);
}

void
roomSimulation::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    _roomDialogs->mouseClicked(event);
    update();
}

void
roomSimulation::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    if ( _roomDialogs->mouseReleased(event) )
    {
        QPointF viewCord = mapToScene(event->pos());
        int xPos = viewCord.x();
        int yPos = viewCord.y();

        if (_room_scene->sceneRect().contains(xPos, yPos) == false)
        {
           _roomDialogs->sendAlertBox("Please click inside the boundaries");
           return;
        }

        roomAtom* curAtom = findClosePoint(xPos, yPos);
        std::cout << "Room Simulation <User Atom Select> the info of atom will be printed " << std::endl;
        curAtom->print();
        if (_roomDialogs->createDialog(curAtom) == 1)
        {
          if(curAtom->getInfo().getType() == STypes::SOURCE_NOICE)
          {
              getAllNoicesArc(curAtom);
          }
        }
    }
    update();
    }

void
roomSimulation::drawMiddleAxisGraph()
{
    drawYAxisGraph(_room_scene->sceneRect().center().x(), false);
    //drawYAxisGraph(_room_scene->sceneRect().center().x(), true);
}


void
roomSimulation::drawYAxisGraph(int xAxisCordinates, bool lowPassFilter )
{
    std::vector<std::pair <double, double> > yAxisAmplitudes;
    _sharpPlot2 = new sharpPlot("YAxisGraph", "AmplitudeSum", lowPassFilter);
    for (int i = _room_scene->sceneRect().top(); i < _room_scene->sceneRect().bottom(); i +=  _roomParameters.pixel4EachAtom)
    {
        roomAtom *atom = qgraphicsitem_cast<roomAtom*>(_room_scene->itemAt(xAxisCordinates, i, QTransform()));
        yAxisAmplitudes.push_back(std::make_pair((i - _room_scene->sceneRect().top()) * _roomParameters.pixel2RealRatio, atom->getResult().second));
        //atom->setOutput(true);
    }
    this->scene()->update();
    this->viewport()->repaint();
    _sharpPlot2->drawBasicGraph(yAxisAmplitudes);
    _sharpPlot2->update();

}


