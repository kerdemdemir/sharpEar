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

#ifndef ENVIRONMENTSETUP_H
#define ENVIRONMENTSETUP_H

#include <QWidget>
#include <utility/commons.h>

/*There are some statics in UI this class is a helper to mainWindow does it initializings*/
/*It didn't grow too much not so many comments needed */

class interActionManager;
class QLayout;     // Forward declarations
class QTableWidget;
class QLineEdit;
class QCheckBox;
class QLabel;     // Forward declarations
class QPushButton;
class QDockWidget;
class QGridLayout;
class QHBoxLayout;
class QVBoxLayout;

typedef std::pair<int, int>  polar_cordinates;
typedef std::pair<double, double>  dist_val_pair;

class enviromentSetup : public QWidget
{
    Q_OBJECT

public:

    enviromentSetup(QWidget *parent);
    void insertDataToTable(std::pair <int, int> cordinate, std::string& fileName);

private Q_SLOTS:
    void reDrawEnvironment();
    void testFilesTableDoubleClicked(int row, int column);
    void startVisulution();


private :

    interActionManager* hndl_interActionManager;

    valuesBasicUserDialog st_environmentParamaters;
    std::vector<std::map<polar_cordinates, double > >space2Values;
    std::vector<polar_cordinates> cordinates4Draw;

    void createDialogs();
    void normalizeEnvironment();
    void getValuesFromDialogs();
    void getSourceIndexFromSounds();
    double calcMicCord(double i);
    double calcMicCord4Calc(double micIndex);
    void createCordinates();
    void calcValues4Cordinates(polar_cordinates& cordinates);
    void setMicArrayBasics();
    void calculateOffset();
    void reset();

    std::pair <int, int> getSourceCordinateFromSounds(int index);

    void setTables();

    size_t _rowCountSoundInput;
    ///******* UI elements **********//

    QLabel * dx_dy;
    QLabel * channelNumber;
    QLabel * channelDistance;
    QLabel * listenRange;

    QLineEdit * inputDx_Dy;
    QLineEdit * inputChannelNumber;
    QLineEdit * inputChannelDistance;
    QLineEdit    * inputListenRange;

    QCheckBox   * checkBoxIsListen;
    QPushButton * maxElementsButton;
    QPushButton * resizeButton;

    QGridLayout *gridLayout;
    QHBoxLayout *outerLayout;
    QVBoxLayout *verticalOuterLayout;

    QTableWidget* sourceListText;
    QTableWidget* testFilesTable;
    QTableWidget* listenListText;

    QStringList sourceListHeadNames;
    QStringList outputTableHeadNames;

    ///********* UI elements end ********//

    ///** Copying not allowed **///
    enviromentSetup(const enviromentSetup*&);
    enviromentSetup& operator= (const enviromentSetup*&);

    void addDefaultFileNames();
};

#endif // ENVIRONMENTSETUP_H
