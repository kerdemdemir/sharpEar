/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionFocusSelect;
    QAction *actionPulse;
    QAction *actionSpeakerTracking;
    QAction *actionScripting_Console;
    QWidget *centralWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuAdvance;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1184, 423);
        actionFocusSelect = new QAction(MainWindow);
        actionFocusSelect->setObjectName(QStringLiteral("actionFocusSelect"));
        actionPulse = new QAction(MainWindow);
        actionPulse->setObjectName(QStringLiteral("actionPulse"));
        actionSpeakerTracking = new QAction(MainWindow);
        actionSpeakerTracking->setObjectName(QStringLiteral("actionSpeakerTracking"));
        actionSpeakerTracking->setCheckable(true);
        actionScripting_Console = new QAction(MainWindow);
        actionScripting_Console->setObjectName(QStringLiteral("actionScripting_Console"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1184, 21));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuAdvance = new QMenu(menuBar);
        menuAdvance->setObjectName(QStringLiteral("menuAdvance"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuAdvance->menuAction());
        menuFile->addSeparator();
        menuAdvance->addAction(actionFocusSelect);
        menuAdvance->addAction(actionPulse);
        menuAdvance->addAction(actionSpeakerTracking);
        menuAdvance->addAction(actionScripting_Console);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "SharpEar", 0));
        actionFocusSelect->setText(QApplication::translate("MainWindow", "FocusSelect", 0));
        actionPulse->setText(QApplication::translate("MainWindow", "Pulse", 0));
        actionSpeakerTracking->setText(QApplication::translate("MainWindow", "SpeakerTracking", 0));
        actionScripting_Console->setText(QApplication::translate("MainWindow", "Scripting Console", 0));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0));
        menuAdvance->setTitle(QApplication::translate("MainWindow", "Advance", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
