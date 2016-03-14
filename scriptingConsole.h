#ifndef SCRIPTINGCONSOLE_H
#define SCRIPTINGCONSOLE_H

#include <QScriptEngine>
#include <QDialog>




namespace Ui {
class ScriptingConsole;
}

class ScriptingConsole : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptingConsole(QWidget *parent = 0);
    ~ScriptingConsole();

    template< typename T >
    void registerWithName( T registerPtr, QString scriptName )
    {
        QScriptValue scriptObj = engine->newQObject(registerPtr);
        QScriptValue global = engine->globalObject();
        global.setProperty(scriptName, scriptObj);
    }

    void evaluate();

    QScriptEngine* getEngine()
    {
        return engine;
    }

private:

    QScriptEngine* engine;
    Ui::ScriptingConsole *ui;
};

#endif // SCRIPTINGCONSOLE_H
