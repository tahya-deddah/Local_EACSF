#ifndef __CSFSCRIPTS__
#define __CSFSCRIPTS__

#include <iostream>

#include <QFile>
#include <QString>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QTextStream>

using namespace std;

class CSFScripts
{
	public:
		CSFScripts();
		~CSFScripts();

		QMap<QString,QString> GetExecutablesMap(){
			return m_Executables;
		}

		QString checkStringValue(QJsonValue str_value){
			if(str_value.isUndefined()){
				return QString("");
			}else{
				return str_value.toString();
			}
		}

		QString checkIntValue(QJsonValue str_value){
			if(str_value.isUndefined()){
				return QString("0");
			}else{
				return QString("%1").arg(str_value.toInt());
			}
		}

		QString checkBoolValue(QJsonValue str_value){
			if(str_value.isUndefined()){
				return QString("false");
			}else{
				return QString("%1").arg(str_value.toBool());
			}
		}

		QString checkDoubleValue(QJsonValue str_value){
			if(str_value.isUndefined()){
				return QString("0");
			}else{
				return QString("%1").arg(str_value.toDouble());
			}
		}

		void setConfig(QJsonObject config);

		void run_EACSF();

		void write_main_script();
		void write_process_left_hemisphere();
		void write_process_right_hemisphere();
		void write_slurm_job();

	    

	    

    private:

    	QJsonObject m_Root_obj;

    	QMap<QString,QString> m_Executables;
    	QMap<QString,QStringList> script_exe;

    	QString m_PythonScripts;
    	
};


#endif