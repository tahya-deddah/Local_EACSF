#include "Local_EACSFCLP.h"
#include "csfwindow.h"

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


int  main(int argc, char** argv) 
{

	PARSE_ARGS;
    QJsonObject root_obj = readConfig(QString::fromStdString(parameters));
   
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