/********************************************************************************
** Form generated from reading UI file 'scriptingConsole.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCRIPTINGCONSOLE_H
#define UI_SCRIPTINGCONSOLE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ScriptingConsole
{
public:
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QTextEdit *textEdit;
    QSpacerItem *verticalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *ScriptingConsole)
    {
        if (ScriptingConsole->objectName().isEmpty())
            ScriptingConsole->setObjectName(QStringLiteral("ScriptingConsole"));
        ScriptingConsole->resize(400, 300);
        verticalLayoutWidget = new QWidget(ScriptingConsole);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 10, 381, 281));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        textEdit = new QTextEdit(verticalLayoutWidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));

        verticalLayout->addWidget(textEdit);

        verticalSpacer = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer);

        pushButton = new QPushButton(verticalLayoutWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        verticalLayout->addWidget(pushButton);


        retranslateUi(ScriptingConsole);

        QMetaObject::connectSlotsByName(ScriptingConsole);
    } // setupUi

    void retranslateUi(QDialog *ScriptingConsole)
    {
        ScriptingConsole->setWindowTitle(QApplication::translate("ScriptingConsole", "Dialog", 0));
        pushButton->setText(QApplication::translate("ScriptingConsole", "Execute", 0));
    } // retranslateUi

};

namespace Ui {
    class ScriptingConsole: public Ui_ScriptingConsole {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCRIPTINGCONSOLE_H
