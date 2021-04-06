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
#include <QFile>
#include <QString>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTextStream>


#include "ui_csfwindow.h"
#include "csfscripts.h"
#include "extexecutableswidget.h"
#include "model.h"
#include "addWidget.h"


namespace Ui{
class CSFWindow;
}

class CSFWindow: public QMainWindow, public Ui::CSFWindow
{

    Q_OBJECT

public:

    explicit CSFWindow(QWidget *parent=0);
    ~CSFWindow();
    QJsonObject getConfig();
    void setConfig(QJsonObject root_obj);


private slots:
    void on_Add_clicked();

private:

    QString OpenFile();
    QString SaveFile();
    QString OpenDir();
    void infoMsgBox(QString message, QMessageBox::Icon type);
    QJsonObject readConfig(QString filename);
    bool writeConfig(QString filename);
    QString checkStringValue(QJsonValue str_value);
    void CleanFile(QString filename);
   


private slots:

    void prc_finished(QProcess *prc, QString outlog_filename, QString errlog_filename, int row);
    void disp_output(QProcess *prc, QString outlog_filename);
    void disp_err(QProcess *prc, QString errlog_filename);

    //File
    void on_actionSave_Config_File_triggered();
    void on_actionLoad_Config_File_triggered();
    //Window menu
    void on_actionToggle_advanced_mode_toggled(bool toggled);
    //About
    void on_actionAbout_triggered();

    //1st tab
    void on_Data_Directory_clicked();
    void on_Find_clicked();   
    bool Find_Paths(QDir odir, QString filter, QStringList &vect);
    void on_Remove_clicked();
    bool ModelIsEmpty();
    void on_batch_slurm_clicked();
    void on_batch_local_clicked();
    void on_Run_Batch_Process_clicked();
    
    
    //2nd tab
   
    void on_T1_clicked();
    void on_Segmentation_clicked();
    void on_CSF_Probability_Map_clicked();
    void on_LH_MID_Surface_clicked();
    void on_LH_GM_Surface_clicked();
    void on_RH_MID_Surface_clicked();
    void on_RH_GM_Surface_clicked();
    void on_output_directory_clicked();

    //3rd tab
    void updateExecutables(QString exeName, QString path);

    //4th tab
    void on_slurm_clicked();
    void on_local_clicked();
    void on_slurm_stateChanged(int state);
    void on_smooth_stateChanged(int state);

    //5th tab
    void on_Execute_clicked();
    void run_Local_EACSF(int row);

    //6th tab
    void on_output_path_clicked();
    void on_itksnap_clicked();
    void on_shapepopoulationviewer_clicked();
    void on_visualize_clicked();
    
private:

    
    QJsonObject root_obj ;
    ExtExecutablesWidget* m_exeWidget;
    static const QString m_github_url;
    CsvTableModel *model ;
    bool batch_processing;
    QList<QStringList> model_data;
    
};

#endif
