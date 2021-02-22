
#ifndef CSFWINDOW_H
#define CSFWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore>
#include <QMap>
#include <QProcess>
#include <QMessageBox>
#include <QTranslator>
#include <iostream>
#include <QFile>
#include <QString>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>
#include <QPixmap>
#include <QRegularExpressionMatch>
#include "ui_csfwindow.h"
#include "csfscripts.h"
#include "extexecutableswidget.h"


#include <QMainWindow>
#include <QWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QtCore>
#include <QMap>
#include <QProcess>
#include <QMessageBox>


namespace Ui{
class CSFWindow;
}

//Declaration of myWindow
class CSFWindow: public QMainWindow, public Ui::CSFWindow
{

    Q_OBJECT

public:

    explicit CSFWindow(QWidget *parent=0);
    ~CSFWindow();
    QJsonObject getConfig();
    void setConfig(QJsonObject root_obj);


private:

    QString OpenFile();
    QString SaveFile();
    QString OpenDir();
    void infoMsgBox(QString message, QMessageBox::Icon type);
    QJsonObject readConfig(QString filename);
    bool writeConfig(QString filename);
    QString checkStringValue(QJsonValue str_value);
   


private slots:

    void prc_finished(int exitCode, QProcess::ExitStatus exitStatus);
    void disp_output(QString output_dir);
    void disp_err(QString output_dir);

    void on_actionSave_Config_File_triggered();
    void on_actionLoad_Config_File_triggered();

    void updateExecutables(QString exeName, QString path);

    void on_T1_clicked();
    void on_Segmentation_clicked();
    void on_CSF_Probability_Map_clicked();
    void on_LH_MID_Surface_clicked();
    void on_LH_GM_Surface_clicked();
    void on_RH_MID_Surface_clicked();
    void on_RH_GM_Surface_clicked();
    void on_output_directory_clicked();
    void on_Execute_clicked();
    void on_Visualize_clicked();
    void display();

    
private:

   
    QProcess *prc;
    QProcess *visualization;
    QJsonObject root_obj ;
    ExtExecutablesWidget* m_exeWidget;

    
};

#endif
