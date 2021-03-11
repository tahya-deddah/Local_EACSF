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
    parametres->setEnabled(false);
    Core->setEnabled(false);
    Memory->setEnabled(false);
    Node->setEnabled(false);
    Time->setEnabled(false);
    lineEdit_Core->setEnabled(false);
    lineEdit_Memory->setEnabled(false);
    lineEdit_Node->setEnabled(false);
    lineEdit_Time->setEnabled(false);


    QJsonObject param_obj = root_obj["parameters"].toObject();
    lineEdit_Core->setText(param_obj["Slurm_cores"].toString());
    lineEdit_Node->setText(param_obj["Slurm_nodes"].toString());
    lineEdit_Time->setText(param_obj["Slurm_time"].toString());
    lineEdit_Memory->setText(param_obj["Slurm_memory"].toString());

    ClosingRadius->setValue(param_obj["Closing_radius"].toInt());
    DilationRadius->setValue(param_obj["Dilation_radius"].toInt());
    IterationsNumber->setValue(param_obj["Iterations_number"].toInt());
    ImageDimension->setText(param_obj["Image_dimension"].toString());
    tab->removeTab(2);

    
   
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

    QJsonObject param_obj = root_obj["parameters"].toObject();
    lineEdit_Core->setText(param_obj["Slurm_cores"].toString());
    lineEdit_Node->setText(param_obj["Slurm_nodes"].toString());
    lineEdit_Time->setText(param_obj["Slurm_time"].toString());
    lineEdit_Memory->setText(param_obj["Slurm_memory"].toString());
    ClosingRadius->setValue(param_obj["Closing_radius"].toInt());
    DilationRadius->setValue(param_obj["Dilation_radius"].toInt());
    IterationsNumber->setValue(param_obj["Iterations_number"].toInt());
    ImageDimension->setText(param_obj["Image_dimension"].toString());

    QJsonArray exe_array = root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        cout<<exe_obj["path"].toString().toStdString()<<endl;

        this->updateExecutables(exe_obj["name"].toString(), exe_obj["path"].toString());
    }

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

    QJsonObject param_obj;
    
    if(slurm->isChecked())
    {
        param_obj["Slurm_nodes"] = lineEdit_Node->text();    
        param_obj["Slurm_cores"] = lineEdit_Core->text();
        param_obj["Slurm_time"] = lineEdit_Time->text();
        param_obj["Slurm_memory"] = lineEdit_Memory->text();
    }

    param_obj["Closing_radius"] = ClosingRadius->value();    
    param_obj["Dilation_radius"] = DilationRadius->value();
    param_obj["Iterations_number"] = IterationsNumber->value();
    param_obj["Image_dimension"] = ImageDimension->text();

    QJsonObject root_obj;
    root_obj["data"] = data_obj;
    root_obj["parameters"] = param_obj;
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

// Window menu

void CSFWindow::on_actionToggle_advanced_mode_toggled(bool toggled)
{
    if (toggled)
       {
           tab->insertTab(2,tab_parametres,QString("Parametres"));

       }
       else
       {
           tab->removeTab(2);

       }
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


    QString exit_message;
    

    if (local->isChecked())
    {
       /* exit_message = QString("Local_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed"));
        if (exitCode==0)
        {
            QString succes = QString("Your python script is now running in the background ");
            exit_message="<font color=\"green\"><b>"+exit_message+"</b></font>";
            output->append(exit_message);
            output->append(succes);
            cout<<exit_message.toStdString()<<endl;
        }
        else
        {
            exit_message="<font color=\"red\"><b>"+exit_message+"</b></font>";
            error->append(exit_message);
            cout<<exit_message.toStdString()<<endl;
        }

        QString outlog_filename = QDir::cleanPath(data_obj["Output_Directory"].toString() + QString("/output_log.txt"));
        QFile outlog_file(outlog_filename);
        outlog_file.open(QIODevice::WriteOnly);
        QTextStream outlog_stream(&outlog_file);
        outlog_stream << output->toPlainText();
        outlog_file.close();

        QString errlog_filename = QDir::cleanPath(data_obj["Output_Directory"].toString() + QString("/errors_log.txt"));
        QFile errlog_file(errlog_filename);
        errlog_file.open(QIODevice::WriteOnly);
        QTextStream errlog_stream(&errlog_file);
        errlog_stream << error->toPlainText();
        errlog_file.close();  */
    }

   

    if (slurm->isChecked())
    {
    
        exit_message = QString("Local_EACSF pipeline ") + ((exitStatus == QProcess::NormalExit) ? QString("exited with code ") + QString::number(exitCode) : QString("crashed")) ;
        if (exitCode==0)
        {
            QString succes = QString("Your python script is now running in the background ");
            exit_message="<font color=\"green\"><b>"+exit_message+"</b></font>";
            output->append(exit_message);
            output->append(succes);
            cout<<exit_message.toStdString()<<endl;
        }
        else
        {
            exit_message="<font color=\"red\"><b>"+exit_message+"</b></font>";
            error->append(exit_message);
            cout<<exit_message.toStdString()<<endl;
        }

        
     }

   


}


void CSFWindow::disp_output(QProcess *prc)
{
    QString output_log(prc->readAllStandardOutput());
    output->append(output_log);
}

void CSFWindow::disp_err(QProcess *prc)
{       

    QString errors(prc->readAllStandardError());
    error->append(errors);
}


void CSFWindow::on_local_clicked()
{
    if (local->isChecked()){slurm->setEnabled(false);}
    else{slurm->setEnabled(true);}
}

void CSFWindow::on_slurm_clicked()
{
    if (slurm->isChecked()){local->setEnabled(false);}
    else{local->setEnabled(true);}

}

void CSFWindow::on_slurm_stateChanged(int state)
{
    bool enab;
        if (state==Qt::Checked){enab=true;}
        else{enab=false;}

        parametres->setEnabled(enab);
        Core->setEnabled(enab);
        Memory->setEnabled(enab);
        Node->setEnabled(enab);
        Time->setEnabled(enab);
        lineEdit_Core->setEnabled(enab);
        lineEdit_Memory->setEnabled(enab);
        lineEdit_Node->setEnabled(enab);
        lineEdit_Time->setEnabled(enab);
}


void CSFWindow::on_Execute_clicked()
{  
    QProcess *prc; 
    prc = new QProcess;

    QJsonObject root_obj = getConfig();

    CSFScripts csfscripts;
    csfscripts.setConfig(root_obj);
    csfscripts.run_EACSF();

    QJsonObject data_obj = root_obj["data"].toObject();
    QJsonObject param_obj = root_obj["parameters"].toObject();
    QString output_dir = data_obj["Output_Directory"].toString();
    QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));

    if (local->isChecked())
    {
        QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
        QStringList params = QStringList() << main_script;

        // connect(prc, &QProcess::readyReadStandardOutput, [prc, this](){
        //    disp_output(prc);
        // });
        // connect(prc, &QProcess::readyReadStandardError, [prc, this](){
        //    disp_err(prc);
        // });
        
        prc->setStandardOutputFile(QDir::cleanPath(output_dir + QString("/output.txt")));
        prc->setStandardErrorFile(QDir::cleanPath(output_dir + QString("/errors.txt")));
        prc->setWorkingDirectory(output_dir);
        
        QMap<QString, QString> executables = m_exeWidget->GetExeMap();
        
        prc->start(executables[QString("python3")], params);
      
        QMessageBox::information(
            this,
            tr("Local EACSF"),
            tr("Is running.")
        );         
    }
    
    if (slurm->isChecked())
    {
        QString slurm_script = QDir::cleanPath(scripts_dir + QString("/slurm-job"));
        QString time = QString("--time=") + param_obj["Slurm_time"].toString();
        QString memory = QString("--mem=") + param_obj["Slurm_memory"].toString();
        QString core = QString("--ntasks=") + param_obj["Slurm_cores"].toString();
        QString node = QString("--nodes=") + param_obj["Slurm_nodes"].toString();

        QStringList params = QStringList() << time << memory << core << node << slurm_script;
        prc->setWorkingDirectory(output_dir);
        connect(prc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(prc_finished(int, QProcess::ExitStatus)));
        prc->start(QString("sbatch"), params);
      
        QMessageBox::information(
            this,
            tr("Local EACSF"),
            tr("Is running.")
        );
    }
   

   
}
