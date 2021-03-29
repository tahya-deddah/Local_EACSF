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

    void prc_finished(QProcess *prc, QString outlog_filename, QString errlog_filename);
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
    void on_Find_clicked();
    void on_Data_Directory_clicked();
    void Find_Paths(QDir odir, QString filter, QVector <QString> &vect);
    void Create_CSV_file(QVector<QString> &v1, QVector<QString>  &v2, QVector<QString>  &v3, QVector<QString> &v4, QVector<QString> &v5, QVector<QString>  &v6, QVector<QString>  &v7);
    void Importe_CSV_file_to_tablewidget(QString csv_file);
    void on_Refresh_clicked();
    
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

    //6th tab
    void on_output_path_clicked();
    void on_itksnap_clicked();
    void on_shapepopoulationviewer_clicked();
    void on_visualize_clicked();
    
private:

    
    QJsonObject root_obj ;
    ExtExecutablesWidget* m_exeWidget;
    static const QString m_github_url;
    
    /*std::vector<std::string> T1_vect;
    std::vector<std::string> Seg_vect;
    std::vector<std::string> CSF_Prob_vect;*/

    /*QVector T1_vect;
    QVector Seg_vect;
    QVector CSF_Prob_vect;*/
    ///////////

    
};

#endif
