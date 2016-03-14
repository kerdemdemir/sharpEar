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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <deque>
#include <QSettings>
#include "scriptingConsole.h"

class QWidget;
class QLayout;
class QDockWidget;
class QtGui;
class QGroupBox;
class QTableWidget;
class QLineEdit;
class QGroupBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QApplication;
class interActionManager;
class enviromentSetup;
class outputDialogs;

//qScriptRegisterMetaType(MainWindow);



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    template< typename T >
    void scriptRegisterWithName( T* registerPtr, QString scriptName)
    {
        console->registerWithName(registerPtr, scriptName);
    }

private:

    interActionManager* hndl_interActionManager;

    QDockWidget* dialogDockWidget;
    enviromentSetup *dialogWidget;
    outputDialogs   *outputWidget;
    outputDialogs   *processedOutputWidget;
    QWidget* roomWidget;
    QDockWidget* outputDockWidget;
    QDockWidget* outputProcessedDockWidget;
    QDockWidget* graphsWidget;

    void setPanels();
    Ui::MainWindow *ui;


    void addNewRecentFile( QString newFileName );
    QList<QString> recentFileNames;
    QList<QAction*> recentFileAction;
    QSettings* settings;
    ScriptingConsole* console = nullptr;

public slots:
    void openFile();
    void on_actionFocusSelect_triggered();

private slots:
    void on_actionSpeakerTracking_triggered(bool checked);
    void on_actionScripting_Console_triggered();
};

#endif // MAINWINDOW_H
