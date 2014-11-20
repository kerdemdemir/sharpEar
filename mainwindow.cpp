#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "interactor.h"
#include "iostream"
#include "environment/enviromentSetup.h"
#include "QFileDialog"
#include <QWidget>
#include <QLayout>
#include <QDockWidget>
#include <QtGui>
#include <QGroupBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <interactor.h>
#include <outputDialogs/outputdialogs.h>
#include <QInputDialog>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    hndl_interActionManager(hndl_interActionManager->getDataShareInstance()),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //this->setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    showMaximized();
    setPanels();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPanels()
{
    dialogDockWidget  = new QDockWidget("User Input", this);
    dialogDockWidget->setEnabled(true);
    dialogDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, dialogDockWidget);
    int dialogWidgetRightPoint = dialogDockWidget->geometry().x() + dialogDockWidget->width();
    hndl_interActionManager->createDialogPanel(dialogWidget, dialogDockWidget);

    outputDockWidget  = new QDockWidget("Basic Output", this);
    outputDockWidget->setEnabled(true);
    outputDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, outputDockWidget);
    hndl_interActionManager->createOutputPanel(outputWidget, outputDockWidget, true);

    outputProcessedDockWidget  = new QDockWidget("Processed Output", this);
    outputProcessedDockWidget->setEnabled(true);
    outputProcessedDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, outputProcessedDockWidget);
    tabifyDockWidget(outputProcessedDockWidget, outputDockWidget);
    hndl_interActionManager->createOutputPanel(processedOutputWidget, outputProcessedDockWidget, false);

    //One line but important step which set the room
    centralWidget()->showMaximized();
    QRectF boundingRectForRoom(QPointF(dialogWidgetRightPoint, geometry().y())
                               , QSize(900,900));
    setCentralWidget(hndl_interActionManager->createRoom(boundingRectForRoom));
}


void MainWindow::on_actionOpen_triggered()
{
    auto wavFileName = QFileDialog::getOpenFileName(this,
        tr("Open wav file"), "",
        tr("All Files (*)"));
    if (wavFileName.isNull() || wavFileName.isEmpty())
        return;
    hndl_interActionManager->setWavFileName(std::string(wavFileName.toLocal8Bit().constData()));
}

void MainWindow::on_actionFocusSelect_triggered()
{
    bool ok;
    double focusDistanse = QInputDialog::getDouble(this, tr("Select focus point"),
                                     tr("Please enter a value for focus distance"), 0, 0, 20, 1, &ok);
    std::vector <double> sourceDists;
    if (ok)
    {
        double sourceDistance = QInputDialog::getDouble(this, tr("get source point"),
                                                tr("Enter relative position of source to focus distance if they should be same enter 1"), 1, 0, 10, 2, &ok);

        if (ok)
        {
            int sourceCount = QInputDialog::getInt(this, tr("Source count"),
                                             tr("please select how many source you need"), 0, 0, 20, 1, &ok);

            std::cout << " Source count " << sourceCount << std::endl;
            for (int i = 1; i <= sourceCount; i++)
            {
                 sourceDists.push_back( sourceDistance * i);
                 std::cout << " Focus point: " << focusDistanse << "d " <<  "Source set to " << sourceDists[i - 1] * focusDistanse  << "d " << std::endl;

            }
           hndl_interActionManager->setFocusPoint(focusDistanse, sourceDists);
        }
        else
        {
            std::cout << " Value couldnt get so mouse click will be used , Focus point: " << focusDistanse << std::endl;
            sourceDists.push_back(0);
            hndl_interActionManager->setFocusPoint(focusDistanse, sourceDists);
        }
    }
    else
    {
        std::cout << " Main Window error while getting focus fix it!!" << std::endl;
    }
}
