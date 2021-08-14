#include "csfscripts.h"


CSFScripts::CSFScripts(){
    m_PythonScripts = QString("/PythonScripts");
}

CSFScripts::~CSFScripts(){}


void CSFScripts::run_EACSF()
{
   
    QJsonObject data_obj = m_Root_obj["data"].toObject();

    QDir out_dir = QDir();
    out_dir.mkdir(QDir::cleanPath(checkStringValue(data_obj["Output_Directory"].toString())));
    out_dir.mkdir(QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF")));
    out_dir.mkdir(QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF") + m_PythonScripts));

    
    QFile saveFile(QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + "/LocalEACSF" +  "/Local_EACSF_config.json"));
    saveFile.open(QIODevice::WriteOnly);
    QJsonDocument saveDoc(m_Root_obj);
    saveFile.write(saveDoc.toJson());
    saveFile.close();


    //0. WRITE MAIN_SCRIPT
    write_main_script();

    //1. WRITE PROCESS_LEFT_HEMISPHERE
    write_process_left_hemisphere();

    //2. WRITE PROCESS_RIGHT_HEMISPHERE
    write_process_right_hemisphere();

    //3. WRITE SLURM_JOB
    write_slurm_job();
    
}

void CSFScripts::setConfig(QJsonObject config){
    m_Root_obj = config;
}

void CSFScripts::write_main_script()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile file(QString(":/PythonScripts/main_script.py"));
    file.open(QIODevice::ReadOnly);
    QString script = file.readAll();
    file.close();

    script.replace("@Output_Directory@", checkStringValue(data_obj["Output_Directory"]));

    QJsonArray exe_array = m_Root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        m_Executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    script.replace("@python3_PATH@", checkStringValue(m_Executables["python3"]));
    script.replace("@ComputeCSFVolume_PATH@", checkStringValue(m_Executables["ComputeCSFVolume"]));
    script.replace("@Smooth@", checkBoolValue(param_obj["Smooth"]));
    script.replace("@Use_75P_Surface@", checkBoolValue(param_obj["Use_75P_Surface"]));
    script.replace("@Label@", checkStringValue(data_obj["Label"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF") + m_PythonScripts);
    
    QString main_script = QDir::cleanPath(scripts_dir + QString("/main_script.py"));
    QFile outfile(main_script);
    outfile.open(QIODevice::WriteOnly);
    QTextStream outstream(&outfile);
    outstream << script;
    outfile.close();
}



void CSFScripts::write_process_left_hemisphere()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile file_left_hemisphere(QString(":/PythonScripts/process_left_hemisphere.py"));
    file_left_hemisphere.open(QIODevice::ReadOnly);
    QString script_left_hemisphere = file_left_hemisphere.readAll();
    file_left_hemisphere.close();

    script_left_hemisphere.replace("@Tissu_Segmentation@", checkStringValue(data_obj["Tissu_Segmentation"]));
    script_left_hemisphere.replace("@CSF_Probability_Map@", checkStringValue(data_obj["CSF_Probability_Map"]));
    script_left_hemisphere.replace("@LH_MID_surface@", checkStringValue(data_obj["LH_MID_surface"]));
    script_left_hemisphere.replace("@LH_GM_surface@", checkStringValue(data_obj["LH_GM_surface"]));
    script_left_hemisphere.replace("@Left_Atlas_Surface@", checkStringValue(data_obj["Left_Atlas_Surface"]));
    script_left_hemisphere.replace("@LH_Inflating_Template@", checkStringValue(data_obj["LH_Inflating_Template"]));
    script_left_hemisphere.replace("@Output_Directory@", checkStringValue(data_obj["Output_Directory"]));
    script_left_hemisphere.replace("@Label@", checkStringValue(data_obj["Label"]));


   QJsonArray exe_array = m_Root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        m_Executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    script_left_hemisphere.replace("@CreateOuterImage_PATH@", checkStringValue(m_Executables["CreateOuterImage"]));
    script_left_hemisphere.replace("@CreateOuterSurface_PATH@", checkStringValue(m_Executables["CreateOuterSurface"]));
    script_left_hemisphere.replace("@EditPolyData_PATH@", checkStringValue(m_Executables["EditPolyData"]));
    script_left_hemisphere.replace("@klaplace_PATH@", checkStringValue(m_Executables["EACSFKlaplace"]));
    script_left_hemisphere.replace("@EstimateCortexStreamlinesDensity_PATH@", checkStringValue(m_Executables["EstimateCortexStreamlinesDensity"]));
    script_left_hemisphere.replace("@AddScalarstoPolyData_PATH@", checkStringValue(m_Executables["AddScalarstoPolyData"]));
    script_left_hemisphere.replace("@HeatKernelSmoothing_PATH@", checkStringValue(m_Executables["HeatKernelSmoothing"]));
    script_left_hemisphere.replace("@ComputeAverageMesh_PATH@", checkStringValue(m_Executables["ComputeAverageMesh"]));
    script_left_hemisphere.replace("@FitPlane_PATH@", checkStringValue(m_Executables["FitPlane"]));
    script_left_hemisphere.replace("@ROImean_PATH@", checkStringValue(m_Executables["ROImean"]));


    script_left_hemisphere.replace("@closingradius@", QString::number(param_obj["Closing_radius"].toInt()));
    script_left_hemisphere.replace("@dilationradius@", QString::number(param_obj["Dilation_radius"].toInt()));
    script_left_hemisphere.replace("@NumberIterations@", QString::number(param_obj["Iterations_number"].toInt()));
    script_left_hemisphere.replace("@imagedimension@", checkStringValue(param_obj["Image_dimension"]));
    script_left_hemisphere.replace("@NumberIter@", checkStringValue(param_obj["Smoothing_numberIter"]));
    script_left_hemisphere.replace("@Bandwith@", checkStringValue(param_obj["Smoothing_bandwith"]));
    script_left_hemisphere.replace("@interpolationMargin@", checkStringValue(param_obj["Interpolation_margin"]));

    script_left_hemisphere.replace("@Clean@", checkBoolValue(param_obj["Clean"]));
    script_left_hemisphere.replace("@Smooth@", checkBoolValue(param_obj["Smooth"]));

    script_left_hemisphere.replace("@Interpolated@", checkBoolValue(param_obj["Interpolated"]));
    script_left_hemisphere.replace("@NotInterpolated@", checkBoolValue(param_obj["NotInterpolated"]));


    script_left_hemisphere.replace("@Use_75P_Surface@", checkBoolValue(param_obj["Use_75P_Surface"]));
    script_left_hemisphere.replace("@Compute_regional_CSF_density@", checkBoolValue(param_obj["Compute_regional_CSF_density"]));

    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF") + m_PythonScripts);
    
    QString left_hemisphere_script = QDir::cleanPath(scripts_dir + QString("/process_left_hemisphere.py"));
    QFile outfile_left_hemisphere(left_hemisphere_script);
    outfile_left_hemisphere.open(QIODevice::WriteOnly);
    QTextStream  outstream_left_hemisphere(&outfile_left_hemisphere);
    outstream_left_hemisphere << script_left_hemisphere;
    outfile_left_hemisphere.close();
}


void CSFScripts::write_process_right_hemisphere()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QJsonObject param_obj = m_Root_obj["parameters"].toObject();

    QFile file_right_hemisphere(QString(":/PythonScripts/process_right_hemisphere.py"));
    file_right_hemisphere.open(QIODevice::ReadOnly);
    QString script_right_hemisphere = file_right_hemisphere.readAll();
    file_right_hemisphere.close();

    script_right_hemisphere.replace("@Tissu_Segmentation@", checkStringValue(data_obj["Tissu_Segmentation"]));
    script_right_hemisphere.replace("@CSF_Probability_Map@", checkStringValue(data_obj["CSF_Probability_Map"]));
    script_right_hemisphere.replace("@RH_MID_surface@", checkStringValue(data_obj["RH_MID_surface"]));
    script_right_hemisphere.replace("@RH_GM_surface@", checkStringValue(data_obj["RH_GM_surface"]));
    script_right_hemisphere.replace("@Right_Atlas_Surface@", checkStringValue(data_obj["Right_Atlas_Surface"]));
    script_right_hemisphere.replace("@RH_Inflating_Template@", checkStringValue(data_obj["RH_Inflating_Template"]));
    script_right_hemisphere.replace("@Output_Directory@", checkStringValue(data_obj["Output_Directory"]));
    script_right_hemisphere.replace("@Label@", checkStringValue(data_obj["Label"]));


    QJsonArray exe_array = m_Root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        m_Executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }

    script_right_hemisphere.replace("@CreateOuterImage_PATH@", checkStringValue(m_Executables["CreateOuterImage"]));
    script_right_hemisphere.replace("@CreateOuterSurface_PATH@", checkStringValue(m_Executables["CreateOuterSurface"]));
    script_right_hemisphere.replace("@EditPolyData_PATH@", checkStringValue(m_Executables["EditPolyData"]));
    script_right_hemisphere.replace("@klaplace_PATH@", checkStringValue(m_Executables["EACSFKlaplace"]));
    script_right_hemisphere.replace("@EstimateCortexStreamlinesDensity_PATH@", checkStringValue(m_Executables["EstimateCortexStreamlinesDensity"]));
    script_right_hemisphere.replace("@AddScalarstoPolyData_PATH@", checkStringValue(m_Executables["AddScalarstoPolyData"]));
    script_right_hemisphere.replace("@HeatKernelSmoothing_PATH@", checkStringValue(m_Executables["HeatKernelSmoothing"]));
    script_right_hemisphere.replace("@ComputeAverageMesh_PATH@", checkStringValue(m_Executables["ComputeAverageMesh"]));
    script_right_hemisphere.replace("@FitPlane_PATH@", checkStringValue(m_Executables["FitPlane"]));
    script_right_hemisphere.replace("@ROImean_PATH@", checkStringValue(m_Executables["ROImean"]));


    script_right_hemisphere.replace("@closingradius@", QString::number(param_obj["Closing_radius"].toInt()));
    script_right_hemisphere.replace("@dilationradius@", QString::number(param_obj["Dilation_radius"].toInt()));
    script_right_hemisphere.replace("@NumberIterations@", QString::number(param_obj["Iterations_number"].toInt()));
    script_right_hemisphere.replace("@imagedimension@", checkStringValue(param_obj["Image_dimension"]));
    script_right_hemisphere.replace("@NumberIter@", checkStringValue(param_obj["Smoothing_numberIter"]));
    script_right_hemisphere.replace("@Bandwith@", checkStringValue(param_obj["Smoothing_bandwith"]));
    script_right_hemisphere.replace("@interpolationMargin@", checkStringValue(param_obj["Interpolation_margin"]));

    script_right_hemisphere.replace("@Clean@", checkBoolValue(param_obj["Clean"]));
    script_right_hemisphere.replace("@Smooth@", checkBoolValue(param_obj["Smooth"]));
    script_right_hemisphere.replace("@Interpolated@", checkBoolValue(param_obj["Interpolated"]));
    script_right_hemisphere.replace("@NotInterpolated@", checkBoolValue(param_obj["NotInterpolated"]));
    script_right_hemisphere.replace("@Use_75P_Surface@", checkBoolValue(param_obj["Use_75P_Surface"]));
    script_right_hemisphere.replace("@Compute_regional_CSF_density@", checkBoolValue(param_obj["Compute_regional_CSF_density"]));

  
    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF") + m_PythonScripts);
    
    QString right_hemisphere_script = QDir::cleanPath(scripts_dir + QString("/process_right_hemisphere.py"));
    QFile outfile_right_hemisphere(right_hemisphere_script);
    outfile_right_hemisphere.open(QIODevice::WriteOnly);
    QTextStream outstream_right_hemisphere(&outfile_right_hemisphere);
    outstream_right_hemisphere << script_right_hemisphere;
    outfile_right_hemisphere.close();
}


void CSFScripts::write_slurm_job()
{

    QJsonObject data_obj = m_Root_obj["data"].toObject();
    QString scripts_dir = QDir::cleanPath(checkStringValue(data_obj["Output_Directory"]) + QString("/LocalEACSF") + m_PythonScripts);
   
    QFile file_slurm_job(QString(":/PythonScripts/slurm-job"));
    file_slurm_job.open(QIODevice::ReadOnly);
    QString script_slurm_job = file_slurm_job.readAll();
    file_slurm_job.close();

    QJsonArray exe_array = m_Root_obj["executables"].toArray();
    foreach (const QJsonValue exe_val, exe_array)
    {
        QJsonObject exe_obj = exe_val.toObject();
        m_Executables[exe_obj["name"].toString()] = exe_obj["path"].toString();
    }
    script_slurm_job.replace("@python3_PATH@", checkStringValue(m_Executables["python3"]));  
    script_slurm_job.replace("@script_PATH@", scripts_dir);
   
    QString slurm_job_script = QDir::cleanPath(scripts_dir + QString("/slurm-job"));
    QFile outfile_slurm_job(slurm_job_script);
    outfile_slurm_job.open(QIODevice::WriteOnly);
    QTextStream outstream_slurm_job(&outfile_slurm_job);
    outstream_slurm_job << script_slurm_job;
    outfile_slurm_job.close();
}




