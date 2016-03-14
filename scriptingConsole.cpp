#include "scriptingConsole.h"
#include "ui_scriptingConsole.h"

ScriptingConsole::ScriptingConsole(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScriptingConsole)
{
    ui->setupUi(this);

    engine = new QScriptEngine (this);
    connect( this->ui->pushButton, &QPushButton::pressed, [this]()
    {
        evaluate();
    });
}

ScriptingConsole::~ScriptingConsole()
{
    delete ui;
}


void ScriptingConsole::evaluate()
{
    auto script = ui->textEdit->toPlainText();
    auto result = engine->evaluate( script );

}
