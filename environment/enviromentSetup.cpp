#include "interactor.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include "enviromentSetup.h"
#include "math.h"
#include "QObject"
#include <QHeaderView>
#include <QAbstractItemView>
#include <QWidget>
#include <QLayout>
#include <QtGui>
#include <QTableWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QDockWidget>


enviromentSetup::enviromentSetup(QWidget *parent) : QWidget (parent)
                , hndl_interActionManager(hndl_interActionManager->getDataShareInstance())
{
    st_environmentParamaters.micNumber = 51;
    //st_environmentParamaters.distanceBetweenMic = 5;
    st_environmentParamaters.distanceBetweenMic = 10;
    st_environmentParamaters.listenRange = 2500; //cm
    st_environmentParamaters.dx_dy       = 20;
    _rowCountSoundInput = 0;
    createDialogs();
    hndl_interActionManager->setBasicUserDialogValues(&st_environmentParamaters);
}

//*** This classes main responsibility is to get user inputs below function are for user dialogs ****//

void
enviromentSetup::createDialogs()
{
    channelNumber   = new QLabel (QApplication::translate("leftPanel","channelNumber"));
    channelDistance = new QLabel (QApplication::translate("leftPanel","chanelDistance"));
    listenRange     = new QLabel (QApplication::translate("leftPanel","listenRange"));

    inputChannelNumber = new QLineEdit(QString::number(st_environmentParamaters.micNumber));
    inputChannelDistance = new QLineEdit(QString::number(st_environmentParamaters.distanceBetweenMic));
    inputListenRange     = new QLineEdit(QString::number(st_environmentParamaters.listenRange));

    checkBoxIsListen = new QCheckBox("Listen");
    maxElementsButton = new QPushButton("Beamforming");
    resizeButton = new QPushButton("Resize");


    gridLayout = new QGridLayout();
    gridLayout->addWidget(channelNumber, 0, 0);
    gridLayout->addWidget(inputChannelNumber, 0, 1);
    gridLayout->addWidget(checkBoxIsListen, 0, 2);

    gridLayout->addWidget(channelDistance, 1, 0);
    gridLayout->addWidget(inputChannelDistance, 1, 1);
    gridLayout->addWidget(maxElementsButton, 1, 2);

    gridLayout->addWidget(listenRange, 2, 0);
    gridLayout->addWidget(inputListenRange, 2, 1);
    gridLayout->addWidget(resizeButton, 2, 2);


    outerLayout = new QHBoxLayout();
    outerLayout->addLayout(gridLayout, 1);
    verticalOuterLayout = new  QVBoxLayout(this);
    verticalOuterLayout->addLayout(outerLayout, 1);
    setTables();

    this->setLayout(verticalOuterLayout);
    this->show();



    QObject::connect(resizeButton, SIGNAL(released()), this, SLOT(reDrawEnvironment()));
    QObject::connect(maxElementsButton, SIGNAL(released()), this, SLOT(startBeamforming()));

}

void enviromentSetup::addDefaultFileNames()
{
    int currentRow = 0;
    std::vector < QString > defaultFileNames = {
    QString("audiocheck.net_sin_1000Hz_-3dBFS_5s.wav"),
    QString("audiocheck.net_sin_1000Hz_-3dBFS_10s.wav"),
    QString("196971__margo-heston__i-see-five-lamps-f.wav"),
    QString("196406__margo-heston__the-airplane-is-blue-m.wav"),
    QString("f1lcapae.wav"),
    QString("m1lfisae.wav"),
    };

    for (auto& elem : defaultFileNames)
    {
        testFilesTable->setItem(currentRow, 0, new QTableWidgetItem(elem));
        currentRow++;
    }
}

void enviromentSetup::setTables()
{
    //gridLayout->addWidget();
    sourceListHeadNames << "SourceFile" << "Cordinates" << "IsSource";
    sourceListText  = new QTableWidget(10, 3);
    sourceListText->setHorizontalHeaderLabels(sourceListHeadNames);
    sourceListText->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    sourceListText->setSelectionBehavior(QAbstractItemView::SelectRows);
    sourceListText->setSelectionMode (QAbstractItemView::SingleSelection);
    sourceListText->setMaximumHeight(200);
    verticalOuterLayout->addWidget(sourceListText);
    sourceListText->resizeColumnsToContents();

    // I canceled this feature since I added recent files
//    outputTableHeadNames << "TestFileNames";
//    testFilesTable  = new QTableWidget(10, 1);
//    testFilesTable->setHorizontalHeaderLabels(outputTableHeadNames);
//    testFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
//    testFilesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
//    testFilesTable->setSelectionMode (QAbstractItemView::SingleSelection);
//    testFilesTable->setMaximumHeight(200);
//    verticalOuterLayout->addWidget(testFilesTable);
//    testFilesTable->resizeColumnsToContents();
//    addDefaultFileNames();

    QObject::connect(sourceListText, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(sourceFilesDoubleClick(int, int)));
    //QObject::connect(testFilesTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(testFilesTableDoubleClicked(int, int)));
    //testFilesTable->hide();

}


void
enviromentSetup::testFilesTableDoubleClicked(int row, int column)
{
     auto itemFileName  = testFilesTable->item(row, 0);
     if (itemFileName == NULL)
         return;
     (void) column;
     auto wavFileName = itemFileName->text();
     if (wavFileName.isNull() || wavFileName.isEmpty())
         return;

     wavFileName = (QString("C:\\") + wavFileName);
     hndl_interActionManager->setWavFileName(std::move(wavFileName.toStdString()));
}

void
enviromentSetup::getSourceIndexFromSounds()
{
    if (sourceListText == NULL)
        return;
    std::vector<std::pair<int, int> > tempSourceCordinates;

    for (int rowCount = 0; rowCount < sourceListText->rowCount(); rowCount++)
    {
        auto item  = sourceListText->item(rowCount, 2);
        if (item != NULL && ((item->text() == "Yes") || (item->text() == "yes")) )
        {
            auto cordinates = std::move(getSourceCordinateFromSounds(rowCount));
            tempSourceCordinates.push_back(cordinates);
        }
    }
    if (tempSourceCordinates.empty())
        return;
    else
        return hndl_interActionManager->setCordinatesOfSources(std::move(tempSourceCordinates));
}

std::pair <int, int>
enviromentSetup::getSourceCordinateFromSounds(int index)
{
     auto item  = sourceListText->item(index, 1);
     QString coordinates = item->text();
     QStringList cordinatesList = coordinates.split(",");
     if (cordinatesList.empty())
         return std::make_pair(0, 0);
     return std::make_pair(cordinatesList[0].toInt(), cordinatesList[1].toInt());;
}

void
enviromentSetup::insertDataToTable(std::pair <int, int> cordinate, std::string& fileName)
{
    QString sourceYesNoStr;
    //sourceListText->setRowCount(rowCount + 1);
    sourceListText->setItem(_rowCountSoundInput, 0, new QTableWidgetItem(QString::fromStdString(fileName)));
    std::string coordinateStr = std::to_string(cordinate.first) + "," + std::to_string(cordinate.second);
    sourceListText->setItem(_rowCountSoundInput, 1, new QTableWidgetItem(QString::fromStdString(coordinateStr)));
    sourceListText->setItem(_rowCountSoundInput, 2, new QTableWidgetItem(sourceYesNoStr));
    _rowCountSoundInput++;
    update();
    return;
}

//** Model Data ; This class has one extra responsibility to calculate distances from each point to each microphone
//   Functions below are for distanse calculation and stufflike pixel to cm conversation might move to room class ***//


void
enviromentSetup::getValuesFromDialogs()
{
    st_environmentParamaters.micNumber =
            inputChannelNumber->text().toInt();
    st_environmentParamaters.distanceBetweenMic =
            inputChannelDistance->text().toInt();
    st_environmentParamaters.listenRange =
            inputListenRange->text().toInt();
}

void
enviromentSetup::reDrawEnvironment()
{
    getValuesFromDialogs();
    hndl_interActionManager->reDrawRoom(st_environmentParamaters);
}

void
enviromentSetup::startBeamforming()
{
    hndl_interActionManager->startBeamforming();
}

