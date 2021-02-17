#ifndef EXTEXECUTABLESWIDGET_H
#define EXTEXECUTABLESWIDGET_H

#include <QWidget>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>


#include <iostream>

using namespace std;

class ExtExecutablesWidget : public QWidget 
{

    Q_OBJECT
public:
    explicit ExtExecutablesWidget(QWidget *m_parent = NULL);
    vector<QString> buildInterface(QMap<QString, QString> exeMap); //Builds the interface
    void setExeDir(QString dir); //Sets the default directory for the QFileDialog
    void setExecutable(QString exe, QString path);
    QJsonArray GetExeArray();
    QMap<QString, QString> GetExeMap();
    QString findExecutable(QString, QString);

signals:
   void newExePath(QString exeName,QString path); //Signal emitted when an executable's path has been changed by the user (to be connected to the application)


private:
    QString m_exeDir; //Default directory for the QFileDialog
    QMap<QString, QLineEdit*> m_ExeMap;


private slots:
   void exeQpbTriggered(); //On trigger of a button, opens a dialog and updates the corresponding lineEdit
   void exeLinedTextChanged(QString new_text); //On change of a line edit, gets the info about the sender (button name = executable name) and emit the signal newExePath(QString,QString)
};

#endif // EXTEXECUTABLESWIDGET_H
