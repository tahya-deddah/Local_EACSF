#include "Local_EACSFCLP.h"
#include "csfwindow.h"
#include "csfscripts.h"


#include <QApplication>
#include <QFile>
#include <QJsonObject>
#include <QString>
#include <QFileInfo>
#include <QObject>

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

QJsonObject readConfig(QString filename)
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

QList<QMap<QString, QString>> readCSV(QString filename)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream stream(&file);
    QList<QMap<QString, QString>> data;
    QString separator(","); 
    QStringList keys = stream.readLine().split(separator); // csv header
    while (stream.atEnd() == false)
    {
        QMap<QString, QString> line;
        QStringList record = stream.readLine().split(separator);
        for (int i = 0 ; i < keys.size() ; i ++)
        {
            line.insert(keys.at(i), record.at(i));
        }
        data << line;
    }
    file.close();
    return data;
}

QString checkStringValue(string str_value, QJsonValue str_default)
{
    if(str_value.compare("") == 0){
        if(str_default.isUndefined()){
            return QString("");
        }
        return str_default.toString();
    }else{
        return QString(str_value.c_str());
    }
}

void Execute_One_Case( QJsonObject root_obj)
{
    QJsonObject data_obj = root_obj["data"].toObject();
    QJsonObject param_obj = root_obj["parameters"].toObject();
    QString output_dir = QDir::cleanPath(data_obj["Output_Directory"].toString());
    QFileInfo info = QFileInfo(output_dir);
    if(!info.exists()){
        QDir out_dir = QDir();
        out_dir.mkpath(output_dir);
    }

    CSFScripts csfscripts;
    csfscripts.setConfig(root_obj);
    csfscripts.run_EACSF();

    QString scripts_dir = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/PythonScripts"));
    QString outlog_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/output_log.txt"));
    QString errlog_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF")+ QString("/errors_log.txt"));

    QProcess* prc =  new QProcess;
    prc->setWorkingDirectory(output_dir);

    if(param_obj["Slurm"].toBool())
    {
        QString CSF_volume_filename = QDir::cleanPath(output_dir + QString("/LocalEACSF") + QString("/")  + data_obj["Label"].toString() + QString("_CSFVolume.txt"));
        QFile volume_file(CSF_volume_filename);
        if(volume_file.exists()) 
        {       
            std::cout <<"Compute Local EACSF Density already done" << std::endl;
        } 
        else
        {
            QString slurm_script = QDir::cleanPath(scripts_dir + QString("/slurm-job"));
            QString time = QString("--time=") + param_obj["Slurm_time"].toString();
            QString memory = QString("--mem=") + param_obj["Slurm_memory"].toString();
            QString core = QString("--ntasks=") + param_obj["Slurm_cores"].toString();
            QString node = QString("--nodes=") + param_obj["Slurm_nodes"].toString();
            QString output_file = QString("--output=") + outlog_filename ;
            QString error_file = QString("--error=") + errlog_filename ;

            QStringList params = QStringList() << time << memory << core << node << output_file << error_file << slurm_script;
            prc->start(QString("sbatch"), params);
        }
    }
    else
    {
        QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
        QStringList params = QStringList() << main_script;

        prc->setStandardOutputFile(outlog_filename);
        prc->setStandardErrorFile(errlog_filename);

        QMap<QString, QString> executables = csfscripts.GetExecutablesMap();
        cout<<executables["python3"].toStdString()<<" "<<params.join(" ").toStdString()<<endl;
        prc->start(executables["python3"], params);
        prc->waitForFinished(-1);
        prc->close(); 
    }

}

int  main(int argc, char** argv) 
{

	PARSE_ARGS;
    QJsonObject root_obj = readConfig(QString::fromStdString(parameters));
   
    if (noGUI)
    {
        QJsonObject data_obj = root_obj["data"].toObject();
        QJsonObject param_obj = root_obj["parameters"].toObject();
        if(BatchProcessing)
        {
            QList<QMap<QString, QString>> CSVFile = readCSV(QString::fromStdString(csv_file));    
            for(int i = 0 ; i< CSVFile.size(); i++)
            {
               
                data_obj["Tissu_Segmentation"] = checkStringValue( (CSVFile[i].value("Tissu Segmentation")).toStdString(),  data_obj["Tissu_Segmentation"]);
                data_obj["CSF_Probability_Map"] = checkStringValue( (CSVFile[i].value("CSF Probability Map")).toStdString(),  data_obj["CSF_Probability_Map"]);
                data_obj["LH_MID_surface"] = checkStringValue(  (CSVFile[i].value("LH MID Surface")).toStdString(),  data_obj["LH_MID_surface"]);
                data_obj["RH_MID_surface"] = checkStringValue((CSVFile[i].value("RH MID Surface")).toStdString(),  data_obj["RH_MID_surface"]);

                if(param_obj["Use_75P_Surface"].toBool())
                {
                    data_obj["LH_GM_surface"] = checkStringValue(  (CSVFile[i].value("LH GM Surface")).toStdString(),  data_obj["LH_GM_surface"] );                
                    data_obj["RH_GM_surface"] = checkStringValue(  (CSVFile[i].value("RH GM Surface")).toStdString(),  data_obj["RH_GM_surface"] );
                }
                
                data_obj["Output_Directory"] = checkStringValue((CSVFile[i].value("Output Directory")).toStdString(),   data_obj["Output_Directory"]);
                data_obj["Label"] = checkStringValue((CSVFile[i].value("Label")).toStdString(),   data_obj["Label"]);    
                root_obj["data"]=data_obj;  
                Execute_One_Case(root_obj);            
            }
        }
        else
        {
            Execute_One_Case(root_obj);
        }            
        return EXIT_SUCCESS;  
    }
    else
    {
        QApplication app(argc,argv);
        Q_INIT_RESOURCE(Local_EACSF_Resources);
        CSFWindow csfw;
        if(parameters.compare("") != 0)
        {
            csfw.setConfig(root_obj);    
        }  
        csfw.show();
        return app.exec();
    }
  
}