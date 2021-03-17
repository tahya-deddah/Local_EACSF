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

std::vector<std::pair<std::string, std::string>> read_csv(std::string filename)
{
    std::vector<std::pair<std::string, std::string>> result;
    std::string line, colname;
    std::vector< string > column_1; 
    std::vector< string > column_2;  

    std::ifstream myFile(filename);
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");

    if(myFile.good())
    {
        while(std::getline(myFile, line))
        {
            std::stringstream ss(line);
            std::getline(ss, colname, ',');
            column_1.push_back({colname});
            std::getline(ss, colname, '\n');
            column_2.push_back({colname});
        }  
    }
    int n = column_1.size();
    for (int i=0; i<n; i++) 
    {
        result.push_back( make_pair(column_1[i],column_2[i]) ); 
    }
    myFile.close();
    return result;
}

QString checkStringValue(string str_value, QJsonValue str_default){
    if(str_value.compare("") == 0){
        if(str_default.isUndefined()){
            return QString("");
        }
        return str_default.toString();
    }else{
        return QString(str_value.c_str());
    }
}



int  main(int argc, char** argv) 
{

	PARSE_ARGS;
    QJsonObject root_obj = readConfig(QString::fromStdString(parameters));
   
    if (noGUI)
    {
        std::vector<std::pair<std::string, std::string>> csv = read_csv(csv_file);
        QJsonObject data_obj = root_obj["data"].toObject();
        data_obj["T1"] = checkStringValue(csv[0].second, data_obj["T1"]);
        data_obj["Tissu_Segmentation"] = checkStringValue(csv[1].second, data_obj["Tissu_Segmentation"]);
        data_obj["CSF_Probability_Map"] = checkStringValue(csv[2].second, data_obj["CSF_Probability_Map"]);
        data_obj["LH_MID_surface"] = checkStringValue(csv[3].second, data_obj["LH_MID_surface"]);
        data_obj["LH_GM_surface"] = checkStringValue(csv[4].second, data_obj["LH_GM_surface"]);
        data_obj["RH_MID_surface"] = checkStringValue(csv[5].second, data_obj["RH_MID_surface"]);
        data_obj["RH_GM_surface"] = checkStringValue(csv[6].second,  data_obj["RH_GM_surface"]);
        data_obj["Output_Directory"] = checkStringValue(csv[7].second,  data_obj["Output_Directory"]);
        root_obj["data"]=data_obj;

        QString output_dir = QDir::cleanPath(data_obj["Output_Directory"].toString());
        QFileInfo info = QFileInfo(output_dir);
        if(!info.exists()){
            QDir out_dir = QDir();
            out_dir.mkpath(output_dir);
        }

        CSFScripts csfscripts;
        csfscripts.setConfig(root_obj);
        csfscripts.run_EACSF();
        QProcess* prc =  new QProcess;
        prc->setWorkingDirectory(output_dir);

        if(slrum)
        {
            QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));
            QString slurm_script = QDir::cleanPath(scripts_dir + QString("/slurm-job"));

            QJsonObject param_obj = root_obj["parameters"].toObject();
            QString time = QString("--time=") + param_obj["Slurm_time"].toString();
            QString memory = QString("--mem=") + param_obj["Slurm_memory"].toString();
            QString core = QString("--ntasks=") + param_obj["Slurm_cores"].toString();
            QString node = QString("--nodes=") + param_obj["Slurm_nodes"].toString();

            QStringList params = QStringList() << time << memory << core << node << slurm_script;
            prc->start(QString("sbatch"), params);
            return EXIT_SUCCESS;
        }

        else
        {
            QString scripts_dir = QDir::cleanPath(output_dir + QString("/PythonScripts"));
            QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
            QStringList params = QStringList() << main_script;

            prc->setStandardOutputFile(QDir::cleanPath(output_dir + QString("/output.txt")));
            prc->setStandardErrorFile(QDir::cleanPath(output_dir + QString("/errors.txt")));

            QMap<QString, QString> executables = csfscripts.GetExecutablesMap();
            cout<<executables["python3"].toStdString()<<" "<<params.join(" ").toStdString()<<endl;
            prc->start(executables["python3"], params);
            return EXIT_SUCCESS;
        }
    
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