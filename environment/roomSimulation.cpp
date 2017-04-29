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
#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>
#include <set>

//#define DEBUG_TEST_MODE
#define POINT_ALWAYS_CENTER


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
    atomDatabase = std::make_shared<AtomList>();

    _roomDialogs->setRoomSimulation(this);
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
    Point centerCM = std::make_pair((double)hndl_interActionManager->getBasicUserDialogValues()->listenRange, 0);
    _micArr->setPos(sceneCenterQ);
    _micArr->setNewPos(sceneCenter, centerCM);
}

void
roomSimulation::calcRoomParameters()
{

    _soundParameters.samplesPerSec = 16000; //* UP_SAMPLE_RATE;
     if (ENABLE_UPSAMPLING)
        _soundParameters.samplesPerSec *= UP_SAMPLE_RATE;

    _soundParameters.currentOutputTime = 2 ;
     _soundParameters.amplitude = 0;
    _soundParameters.samplePerOutput = _soundParameters.currentOutputTime * _soundParameters.samplesPerSec;

    _roomParameters.numberOfMics = hndl_interActionManager->getBasicUserDialogValues()->micNumber;
    _roomParameters.distancesBetweenMics = hndl_interActionManager->getBasicUserDialogValues()->distanceBetweenMic;
    _roomParameters.yPixelCount = _room_scene->sceneRect().bottom() - _room_scene->sceneRect().top();
    _roomParameters.pixel2RealRatio = (double)hndl_interActionManager->getBasicUserDialogValues()->listenRange
                                        / (double)_roomParameters.yPixelCount;

    _roomParameters.pixel4EachAtom = 3;
    _roomParameters.angleDist = 1;
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
roomSimulation::reset( )
{
    _micArr->renewBuffers();
    _roomDialogs->clear();

    for ( auto atom : hndl2Atom )
    {
        atom->setRoomParams( _roomParameters );
        atom->setType(STypes::UNDEFINED);
        atom->getInfo().setOutput(false);
    }

    this->scene()->update();
    this->viewport()->repaint();
}

void
roomSimulation::resetAtoms( const roomVariables& _roomParameters)
{
    for ( auto atom : hndl2Atom )
    {
       atom->setRoomParams( _roomParameters );
    }
}

void
roomSimulation::reset( int microphoneNumber )
{
    _roomParameters.numberOfMics = microphoneNumber;
    _micArr->setElemCount(microphoneNumber);
    defaultMicPos();
    _roomDialogs->setRoomParams(_roomParameters);
    resetAtoms(_roomParameters);
}

void
roomSimulation::reset( int microphoneNumber, int distance )
{
    _roomParameters.distancesBetweenMics = distance;
    _micArr->setDistance(distance);
    reset(microphoneNumber);
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
            roomAtom* tempAtom = new roomAtom(_soundParameters, _roomParameters, _micArr);
            _room_scene->addItem(tempAtom);            
            tempAtom->setPos(x, y);
            Point atomScenePos = std::make_pair(x, y);
            Point atomCMPos = std::make_pair((x - _room_scene->sceneRect().left()) * _roomParameters.pixel2RealRatio ,
                                             (y - _room_scene->sceneRect().top()) * _roomParameters.pixel2RealRatio );
            tempAtom->createInfo( atomCMPos, atomScenePos );
            hndl2Atom.push_back(tempAtom);
            atomDatabase->insert(tempAtom->getInfo().getRadius(), tempAtom->getInfo().getAngle(), tempAtom);
            tempAtom->setData(0, "Atom");
         }
    }
    atomDatabase->sort();
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

            roomAtom* tempAtom = new roomAtom(_soundParameters, _roomParameters, _micArr);
            _room_scene->addItem(tempAtom);
            tempAtom->setPos(curPoint.x(), curPoint.y());
            Point atomScenePos = std::make_pair(curPoint.x(), curPoint.y());
            Point atomCMPos = std::make_pair((curPoint.x() - _room_scene->sceneRect().left()) * _roomParameters.pixel2RealRatio,
                                             (curPoint.y() - _room_scene->sceneRect().top()) * _roomParameters.pixel2RealRatio );
            tempAtom->createInfo( atomCMPos, atomScenePos );
            hndl2Atom.push_back(tempAtom);
            atomDatabase->insert(tempAtom->getInfo().getRadius(), tempAtom->getInfo().getAngle(), tempAtom);
            tempAtom->setData(0, "Atom");
        }
    }
    atomDatabase->sort();
}

void
roomSimulation::startBeamforming()
{
//    for (size_t i = 0; i < hndl2Atom.size(); i++)
//    {
//        auto& atomPointer = hndl2Atom[i];
//        std::shared_ptr< std::packaged_task< void() > > tempTask (new std::packaged_task<void()>([&atomPointer]()
//        {
//            atomPointer->start();
//        }));
//        _workerThread.post(tempTask);
//    }
    while (true)
    {
        if (_roomDialogs->process() == -1)
        {
            reset();
            return;
        }
        //threadProcess();
        //startAtomColoring();
        _micArr->renewBuffers();
    }
}

CDataType roomSimulation::getImpulseResponce( CDataType& weights)
{
    SoundInfo in = findAtomPolarImpl(getRoomLen()/2, 0)->getInfo();
    auto listAtom = getAtomInRadius( in.getRadius(), false );


    CDataType pulseData;

    createPulse( pulseData, _soundParameters.samplePerOutput,  _soundParameters.samplesPerSec );
    SoundData<CDataType> data(99, in  );
    data.setInfo().setType( STypes::PULSE);
    data.setData(pulseData.begin(), pulseData.end());

    _micArr->adjustArrayFocus( in, ArrayFocusMode::NO_FOCUS);

    _micArr->feed( data, weights );
    std::vector<double> temp;
    for ( auto atom : listAtom )
    {
        atom->start();
        temp.push_back( atom->getResult().second  );
    }

    auto maxMinPair = std::minmax_element(temp.begin(), temp.end());
    auto min = *maxMinPair.first;
    auto max = *maxMinPair.second;
    CDataType returnVal;
    for ( auto atom : listAtom )
    {
        atom->setColor(false, min, max);
        returnVal.push_back( SingleCDataType(atom->getResult().second)  );
    }

    //for ( int i = 0; i < 90; i++)
      //  returnVal[90 + i] = returnVal[90 - i];
    return returnVal;
}


void
roomSimulation::threadProcess()
{
    _workerThread.process();
    _workerThread.clearTasks();
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
roomSimulation::findAtomPolarImpl( double radius, double angle )
{
    double cosVal  = radius * cos(angle * GLOBAL_PI / 180.0);
    double yPos = _room_scene->sceneRect().top() + (abs(cosVal) / _roomParameters.pixel2RealRatio);
    double sinVal  = radius * sin(angle * GLOBAL_PI / 180.0);
    double xPos = _micArr->getSceneMiddlePos().first + ( sinVal / _roomParameters.pixel2RealRatio );

    return qgraphicsitem_cast<roomAtom*>(_room_scene->itemAt( xPos, yPos, QTransform()));
}

roomAtom*
roomSimulation::findAtomPolarFromDataBase( double radius, double angle )
{
   return atomDatabase->getByAngleRadius( angle, radius);
}

double roomSimulation::getRoomLen()
{
    return _roomParameters.pixel2RealRatio* _roomParameters.yPixelCount;
}

void roomSimulation::listen(double radius, double angle)
{
    auto atom = findAtomPolarImpl( radius, angle );
    _roomDialogs->listen(atom);
    this->scene()->update();
    this->viewport()->repaint();
}

void roomSimulation::insertSound( double radius, double angle, QString soundFileName, QString soundType )
{
    auto atom = findAtomPolarImpl( radius, angle );
    if ( !atom )
        return;
    _roomDialogs->insertSound(atom, soundFileName, soundType);
    this->scene()->update();
    this->viewport()->repaint();
}

roomAtom*
roomSimulation::findClosePoint(int xPos, int yPos)
{
    double closestElement = INT_MAX;
    roomAtom* atom  = nullptr;
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


radAngMultAccess<roomAtom *>
roomSimulation::getArcRadius( roomAtom* arcCenter )
{
    radAngMultAccess<roomAtom *> returnVal;
    SoundInfo soundInfo = arcCenter->getInfo();
    //std::cout << " Room Simulation <getAllNoicesArc> A noiced sound is add so candidate noice points will be set " << std::endl;
    double angle = soundInfo.getAngle() ;
    double radius = soundInfo.getRadius();
    int radiusEpsilon = _roomParameters.pixel4EachAtom * _roomParameters.pixel2RealRatio;
    int radiusElemCount = 200; // 100cm is 1 meter

    for (int curRadius = (radius - radiusElemCount/2);
        curRadius < (radius + radiusElemCount/2);
        curRadius += radiusEpsilon)
    {
       for (int curAngle = angle - 45 ; curAngle < angle + 45; curAngle += 3)
       {
           roomAtom *atom = findAtomPolarImpl( curRadius, curAngle );
           if (atom == NULL)
               continue;

           auto allVec = returnVal.getAllData();
           if (std::find(allVec.begin(), allVec.end(), atom) != allVec.end())
               continue;

           //atom->print();
           returnVal.insert(curRadius, curAngle, atom);
       }
    }

    return returnVal;
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

#ifdef POINT_ALWAYS_CENTER
     if (isFirst)
     {
        _roomDialogs->mouseClicked( _room_scene->sceneRect().center() );
     }
    else
     {
         _roomDialogs->mouseClicked(event->pos());
     }
#else
    _roomDialogs->mouseClicked(event->pos());
#endif
    update();
}

void
roomSimulation::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);
    QPointF clickPos = event->pos();

#ifdef POINT_ALWAYS_CENTER
    if (isFirst)
    {
        clickPos = _room_scene->sceneRect().center ();
    }

#endif

    if ( _roomDialogs->mouseReleased(clickPos) )
    {
        QPointF viewCord = mapToScene(clickPos.toPoint());
        #ifdef POINT_ALWAYS_CENTER
        if (isFirst)
        {
            isFirst = false;
            viewCord = clickPos;
        }
        #endif
        int xPos = viewCord.x();
        int yPos = viewCord.y();

        if (_room_scene->sceneRect().contains(xPos, yPos) == false)
        {
           _roomDialogs->sendAlertBox("Please click inside the boundaries");
           return;
        }
        //roomAtom* curAtom = findClosePoint(xPos, yPos);
        roomAtom* curAtom = qgraphicsitem_cast<roomAtom*>(_room_scene->itemAt( xPos, yPos, QTransform()));
        std::cout << "Room Simulation <User Atom Select> the info of atom will be printed " << std::endl;
        curAtom->print();
        _roomDialogs->createDialog(curAtom);
    }
    update();
    }

void
roomSimulation::drawMiddleAxisGraph()
{
    drawYAxisGraph(_room_scene->sceneRect().center().x(), false);
    //drawYAxisGraph(_room_scene->sceneRect().center().x(), true);
}


std::vector< roomAtom* >
roomSimulation::getAtomsInAngleDataBase( int angle, int jump  )
{
    std::vector< roomAtom* > returnVal = atomDatabase->getByAngle(angle);
    if ( !jump )
        return returnVal;

    std::vector< roomAtom* > tempList = returnVal;
    for ( auto elem : tempList)
    {
        int rad = elem->getRadius();
        size_t key = rad / jump;
        if ( rad > 200 && tempList.size() > key )
            returnVal.push_back( tempList[key]);
    }
    return returnVal;
}

std::vector< roomAtom* >
roomSimulation::getAtomsInAngle( int angle, int jump, bool isUnique  )
{
    std::vector< roomAtom* > returnVal;
    double maxRad = (getRoomLen() / cos( angle * GLOBAL_PI / 180 ));
    std::vector< bool > uniqueSet(  maxRad / jump, false );
    std::vector< roomAtom* > allAtoms;
    for ( auto elem : hndl2Atom )
    {
        if ( !elem )
            continue;
        auto atomStruct = dynamic_cast<roomAtom*>(elem);
        if (atomStruct == NULL)
            continue;
        int rad = atomStruct->getInfo().getRadius();
        if ( rad < 200 || rad > (maxRad - 100) )
            continue;
        double angleTemp = elem->getInfo().getAngle();
        if ( isUnique && angle == std::floor((double)angleTemp + 0.5)  )
            allAtoms.push_back(elem);
        else if ( angleTemp >= angle - 1 && angleTemp <= angle + 1 )
            allAtoms.push_back(elem);
    }

    std::sort(allAtoms.begin(), allAtoms.end(), []( const roomAtom* lhs,  const roomAtom* rhs){
        return lhs->getRadius() < rhs->getRadius();
    });

    for ( auto& atomStruct : allAtoms )
    {
        int rad = atomStruct->getInfo().getRadius();
        int key = rad / jump;
        if ( isUnique && !uniqueSet[key] )
        {
            returnVal.push_back(atomStruct);
            uniqueSet[key] = true;
        }
        else
            returnVal.push_back(atomStruct);
    }

    return returnVal;
}

std::vector< roomAtom* >
roomSimulation::getMiddleAtoms()
{
    std::vector< roomAtom* > returnVal;
    auto xAxisCordinates = _room_scene->sceneRect().center().x();
    for (int i = _room_scene->sceneRect().top();
         i < _room_scene->sceneRect().bottom(); i += _roomParameters.pixel4EachAtom
 )
    {
        roomAtom *atom = qgraphicsitem_cast<roomAtom*>(_room_scene->itemAt(xAxisCordinates, i, QTransform()));
        returnVal.push_back(atom);
    }
    return returnVal;
}

std::vector< roomAtom* >
roomSimulation::getAtomsInRadiusDataBase( int curRadius  )
{
    return atomDatabase->getByRadius(curRadius);
}

std::vector< roomAtom* >
roomSimulation::getAtomInRadius(int curRadius , bool isUnique, int start, int offSet )
{
    if ( start < -90)
        start = -90;
    std::set< int > uniqueSet;

    std::vector< roomAtom* > returnVal;
    for (double curAngle = start ; curAngle < start + offSet; curAngle += 0.5)
    {
       roomAtom *atom = findAtomPolarImpl( curRadius, curAngle );
       if ( !atom )
            continue;
        auto isRoomAtom = dynamic_cast<roomAtom*>(atom);
        if (isRoomAtom == NULL)
            continue;
        if ( isUnique )
        {
            if ( uniqueSet.find(atom->getInfo().getAngle()) != uniqueSet.end())
                continue;
        }

        returnVal.push_back(isRoomAtom);
        uniqueSet.insert(atom->getInfo().getAngle()) ;
    }

    return returnVal;
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

void
roomSimulation::drawAngleValueGraph( )
{
    std::vector<std::pair <double, double> > amplitudes;
    _sharpPlot2 = new sharpPlot("RadiusGraph", "AmplitudeSum", false);
    auto middleAtom = findClosePoint(_room_scene->sceneRect().center().x(), _room_scene->sceneRect().center().y());
    auto curRadius = middleAtom->getInfo().getRadius();

    if (ENABLE_RADIAN_ANGLE)
    {
        for ( auto atom : hndl2Atom)
        {
           if ( atom->getInfo().getRadius() >= curRadius - 2 &&
                atom->getInfo().getRadius() <= curRadius + 2)
               amplitudes.emplace_back(atom->getInfo().getAngle(), atom->getResult().second);
        }
        std::sort(amplitudes.begin(), amplitudes.end(), []( std::pair <double, double> lhs, std::pair <double, double> rhs )
        { return lhs.first < rhs.first;});
    }
    else
    {
        for (double curAngle = -90 ; curAngle < 90; curAngle += 0.5)
        {
            roomAtom *atom = findAtomPolarImpl( curRadius, curAngle );
            if (atom == NULL)
                continue;
            amplitudes.emplace_back(curAngle, atom->getResult().second);
        }
    }
    this->scene()->update();
    this->viewport()->repaint();
    _sharpPlot2->drawBasicGraph(amplitudes);
    _sharpPlot2->update();

}

