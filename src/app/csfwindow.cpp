#include "csfwindow.h"

#include<stdlib.h>
#include <string>
#include <iostream>
#include <QtGui>
#include <QFileDialog>
#include <QBoxLayout>
#include <QTextStream>

#ifndef Local_EACSF_TITLE
#define Local_EACSF_TITLE "Local_EACSF"
#endif

#ifndef Local_EACSF_CONTRIBUTORS
#define Local_EACSF_CONTRIBUTORS "Martin Styner, Juan Prieto, Tahya Deddah"
#endif

const QString CSFWindow::m_github_url = "https://github.com/tahya-deddah/Local_EACSF";



CSFWindow::CSFWindow(QWidget *m_parent)
    :QMainWindow(m_parent)
{
    
    this->setupUi(this);
    prc = new QProcess;
    QJsonObject root_obj = readConfig(QString(":/config/default_config.json"));


    QJsonArray exe_array = root_obj["executables"].toArray();
    QMap<QString, QString> executables;

    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    QBoxLayout* exe_layout = new QBoxLayout(QBoxLayout::LeftToRight, tab_executables);
    m_exeWidget = new ExtExecutablesWidget();

    if (!executables.keys().isEmpty())
    {
        m_exeWidget->buildInterface(executables);
        m_exeWidget->setExeDir(QDir::currentPath());
        exe_layout->addWidget(m_exeWidget,Qt::AlignCenter);
        connect(m_exeWidget, SIGNAL(newExePath(QString,QString)), this, SLOT(updateExecutables(QString,QString)));
    }

}

CSFWindow::~CSFWindow(){}


QJsonObject CSFWindow::readConfig(QString filename)
{
    QString config_qstr;
    QFile config_qfile;
    config_qfile.setFileName(filename);
    config_qfile.open(QIODevice::ReadOnly | QIODevice::Text);
    config_qstr = config_qfile.readAll();
    config_qfile.close();

    QJsonDocument config_doc = QJsonDocument::fromJson(config_qstr.toUtf8()); 
    QJsonObject root_obj = config_doc.object();
    return root_obj;
}

bool CSFWindow::writeConfig(QString filename)
{
    QFile saveFile(filename);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        std::cout<<"Couldn't open save file."<<std::endl;
        return false;
    }

    QJsonDocument saveDoc(getConfig());
    saveFile.write(saveDoc.toJson());

    std::cout<<"Saved configuration : "<<filename.toStdString()<<endl;
    return true;
}


void CSFWindow::setConfig(QJsonObject root_obj)
{
    QJsonObject data_obj = root_obj["data"].toObject();
    lineEdit_T1img->setText(data_obj["T1"].toString()); 
    lineEdit_Segmentation->setText(data_obj["Tissu_Segmentation"].toString());
    lineEdit_CSF_Probability_Map->setText(data_obj["CSF_Probability_Map"].toString());
    lineEdit_LH_MID_Surface->setText(data_obj["LH_MID_surface"].toString());
    lineEdit_LH_GM_Surface->setText(data_obj["LH_GM_surface"].toString());
    lineEdit_RH_MID_Surface->setText(data_obj["RH_MID_surface"].toString());
    lineEdit_RH_GM_Surface->setText(data_obj["RH_GM_surface"].toString());
    lineEdit_Output_Directory->setText(data_obj["Output_Directory"].toString());

}

QJsonObject CSFWindow::getConfig(){

    QJsonObject data_obj; 
    data_obj["T1"]=lineEdit_T1img->text();
    data_obj["Tissu_Segmentation"]=lineEdit_Segmentation->text();
    data_obj["CSF_Probability_Map"]=lineEdit_CSF_Probability_Map->text();
    data_obj["LH_MID_surface"]=lineEdit_LH_MID_Surface->text();
    data_obj["LH_GM_surface"]=lineEdit_LH_GM_Surface->text();
    data_obj["RH_MID_surface"]=lineEdit_RH_MID_Surface->text();
    data_obj["RH_GM_surface"]=lineEdit_RH_GM_Surface->text();
    data_obj["Output_Directory"]=lineEdit_Output_Directory->text();


    QJsonObject root_obj;
    root_obj["data"] = data_obj;
    root_obj["executables"] = m_exeWidget->GetExeArray();
    return root_obj;
}

QString CSFWindow::OpenFile(){
    QString filename = QFileDialog::getOpenFileName(
                this,
                "Open File",
                "C://",
                "All files (*.*);; NIfTI File (*.nii *.nii.gz);; NRRD File  (*.nrrd);; Json File (*.json)"
                );
    return filename;
}

QString CSFWindow::SaveFile()
  {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Document",
        QDir::currentPath(),
        "JSON file (*.json)") ;
    return filename;
  }

QString CSFWindow::OpenDir(){
    QString dirname = QFileDialog::getExistingDirectory(
                this,
                "Open Directory",
                "C://",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                );
    return dirname;
}




  QString CSFWindow::checkStringValue(QJsonValue str_value)
{
            if(str_value.isUndefined()){
                return QString("");
            }else{
                return str_value.toString();
            }
}


void CSFWindow::infoMsgBox(QString message, QMessageBox::Icon type)
{
    QMessageBox mb;
    mb.setIcon(type);
    mb.setText(message);
    mb.setStandardButtons(QMessageBox::Ok);
    mb.exec();
}


//File menu

void CSFWindow::on_actionLoad_Config_File_triggered()
{

        QString filename= OpenFile();
        if (!filename.isEmpty())
        {
            setConfig(readConfig(filename));
        }

}

void CSFWindow::on_actionSave_Config_File_triggered()
{
        QString filename=SaveFile();
        if (!filename.isEmpty())
        {
            if (!filename.endsWith(QString(".json")))
            {
                filename+=QString(".json");
            }
            bool success=writeConfig(filename);
            if (success)
            {
                infoMsgBox(QString("Configuration saved with success : ")+filename,QMessageBox::Information);
            }
            else
            {
                infoMsgBox(QString("Couldn't save configuration file at this location. Try somewhere else."),QMessageBox::Warning);
            }
        }

}

//Help menu


void CSFWindow::on_actionAbout_triggered()
{
    QString messageBoxTitle = "About " + QString( Local_EACSF_TITLE );
           QString aboutFADTTS;
           aboutFADTTS = "<b>Contributors:</b> " + QString( Local_EACSF_CONTRIBUTORS ) + "<br>"
                  "<b>Github:</b> <a href=" + m_github_url + ">Click here</a><br>";
       QMessageBox::information( this, tr( qPrintable( messageBoxTitle ) ), tr( qPrintable( aboutFADTTS ) ), QMessageBox::Ok );
}



// 1st Tab - Input

void CSFWindow::on_T1_clicked()
{

    QString path=OpenFile();

    if (!path.isEmpty())
    {
        lineEdit_T1img->setText(path);
    }
}

void CSFWindow::on_Segmentation_clicked()
{

    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_Segmentation->setText(path);
    }
}

void CSFWindow::on_CSF_Probability_Map_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
       lineEdit_CSF_Probability_Map->setText(path);
    }
}

void CSFWindow::on_LH_MID_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_MID_Surface->setText(path);
    }

}

void CSFWindow::on_LH_GM_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_LH_GM_Surface->setText(path);
    }
}

void CSFWindow::on_RH_MID_Surface_clicked()
{

    QString path=OpenFile();
    if (!path.isEmpty())
    {
        lineEdit_RH_MID_Surface->setText(path);
    }

}

void CSFWindow::on_RH_GM_Surface_clicked()
{
    QString path=OpenFile();
    if (!path.isEmpty())
    {
       lineEdit_RH_GM_Surface->setText(path);
    }

}
void CSFWindow::on_output_directory_clicked()
{
   QString path=OpenDir();
    if (!path.isEmpty())
    {
        lineEdit_Output_Directory->setText(path);
    }
}


// 2nd Tab - Executables tab
void CSFWindow::updateExecutables(QString exeName, QString path)
{
   m_exeWidget->setExecutable(exeName, path);
}



// 3rd Tab - Execution

void CSFWindow::prc_finished(int exitCode, QProcess::ExitStatus exitStatus){

    QJsonObject root_obj = getConfig();

    QJsonObject data_obj = root_obj["data"].toObject();
    QString output_dir = data_obj["output_dir"].toString();

    QString exit_message;
    exit_message = QString("Local_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed"));
    if (exitCode==0)
    {
        exit_message="<font color=\"green\"><b>"+exit_message+"</b></font>";
    }
    else
    {
        exit_message="<font color=\"red\"><b>"+exit_message+"</b></font>";
    }
    output->append(exit_message);
    std::cout<<exit_message.toStdString()<<std::endl;

}


void CSFWindow::disp_output(QString output_dir)
{

    QFile Output_filename(QDir::cleanPath(output_dir + QString("/Output_filename.txt"))); 
    Output_filename.open(QIODevice::ReadOnly);
    QTextStream out(&Output_filename); 
    while (!out.atEnd())
        {  
            QString line = out.readLine(); 
            output->append(line); 
            qDebug()<<line ;
        }   
}

void CSFWindow::disp_err(QString output_dir)
{       

    QFile Output_filename(QDir::cleanPath(output_dir + QString("/Errors_filename.txt"))); 
    Output_filename.open(QIODevice::ReadOnly);
    QTextStream out(&Output_filename); 
    while (!out.atEnd())
        {   
            QString line = out.readLine(); 
            error->append(line); 
            qDebug()<<line ;
        }   
}


void CSFWindow::on_Execute_clicked()
{
    QJsonObject root_obj = getConfig();

    CSFScripts csfscripts;
    csfscripts.setConfig(root_obj);
    csfscripts.run_EACSF();

    QJsonObject data_obj = root_obj["data"].toObject();

    QString output_dir = data_obj["Output_Directory"].toString();
    QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));
    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    QStringList params = QStringList() << main_script;

    prc->setStandardOutputFile(QDir::cleanPath(output_dir + QString("/Output_filename.txt")));
    prc->setStandardErrorFile(QDir::cleanPath(output_dir + QString("/Errors_filename.txt")));

    connect(prc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(prc_finished(int, QProcess::ExitStatus)));
   
    prc->setWorkingDirectory(output_dir);
    
    QMap<QString, QString> executables = m_exeWidget->GetExeMap();
    
    prc->start(executables[QString("python3")], params);
  
    QMessageBox::information(
        this,
        tr("Auto EACSF"),
        tr("Is running.")
    );

// Display the output and errors in the interface
    prc->waitForFinished();
    disp_output(output_dir);
    disp_err(output_dir);

}






