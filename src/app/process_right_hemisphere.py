import argparse
import time
import sys
import os
import shutil 
from shutil import copyfile
import itk
import vtk
import subprocess
from subprocess import Popen


def call_and_print(args):

	exe_path = args[0]
	print(" ".join(args))
	    
	completed_process = subprocess.run(args, capture_output=True, text=True)
	   
	status_code = completed_process.returncode
	out = completed_process.stdout
	err = completed_process.stderr
	check_returncode = completed_process.check_returncode()

	print("status code is:",status_code)
	if err!="":
		print(err)
	if out!="":
		print("output message :",out)
	if status_code != 0:
	   	print(check_returncode)



def main_loop(args):
	start = time.time()
	print ("Processing Right Side")
	process_RH(args) 
	end = time.time()
	print("time for RH:",end - start)

def process_RH(args):
	RH_Directory = os.path.join(args.Output_Directory,"RH_Directory") 
	isDir = os.path.isdir(RH_Directory)
	if isDir==False :
		os.mkdir(RH_Directory) 
		print("RH Directory created") 
	else :
		print ("Dirctory exist")

	# Copying Data to the output Directory 
	T1 = os.path.join(RH_Directory, "T1.nrrd")
	Data_existence_test =os.path.isfile(T1) 
	if Data_existence_test==False:
		print("Copying Data")
		copyfile(args.T1, os.path.join(RH_Directory,"T1.nrrd"))
		copyfile(args.Tissu_Segmentation, os.path.join(RH_Directory,"Tissu_Segmentation.nrrd"))
		copyfile(args.CSF_Probability_Map, os.path.join(RH_Directory,"CSF_Probability_Map.nrrd"))
		copyfile(args.RH_MID_surface, os.path.join(RH_Directory,"RH_MID.vtk"))
		copyfile(args.RH_GM_surface, os.path.join(RH_Directory,"RH_GM.vtk"))
		print("Copying Done")
	else :
		print("Data Exists")	

	#Executables:
	CreateOuterImage = args.CreateOuterImage
	CreateOuterSurface = args.CreateOuterSurface
	EditPolyData = args.EditPolyData
	klaplace = args.klaplace
	EstimateCortexStreamlinesDensity = args.EstimateCortexStreamlinesDensity
	AddScalarstoPolyData = args.AddScalarstoPolyData


	###
	os.chdir(RH_Directory)
	Streamline_Path = os.path.join(RH_Directory,"RH_Outer_streamlines.vtk")
	StreamlinesExistenceTest = os.path.isfile(Streamline_Path)
	if StreamlinesExistenceTest ==True :
		print('RH streamline computation Done!')
	else:
		print('Creating Outer RH Convex Hull Surface')
		print('Creating RH Outer Image')
		call_and_print([CreateOuterImage,"--InputImg", "Tissu_Segmentation.nrrd", "--OutputImg","RH_GM_Dilated.nrrd", "--closingradius",'15', "--dilationradius",'2', "--Reverse",'1'])
		print('Creating RH Outer Surface')
		call_and_print([CreateOuterSurface, "--InputBinaryImg", "RH_GM_Dilated.nrrd", "--OutputSurface", "RH_GM_Outer_MC.vtk", "--NumberIterations", '1'])
		call_and_print([EditPolyData, "--InputSurface", "RH_GM_Outer_MC.vtk", "--OutputSurface", "RH_GM_Outer_MC.vtk", "--flipx", ' -1', "--flipy", ' -1', "--flipz", '1'])
		print('Creating Outer RH Convex Hull Surface Done!')

		print('Creating RH streamlines')
		print('CEstablishing Surface Correspondance')
		call_and_print([klaplace,'-dims','300',"RH_MID.vtk", "RH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"RH_Outer.corr"])

		print('CEstablishing Streamlines')
		call_and_print([klaplace, '-traceStream',"RH_Outer.corr_field.vts","RH_MID.vtk", "RH_GM_Outer_MC.vtk", "RH_Outer_streamlines.vtp", \
									"RH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"RH_Outer_streamlines.vtp", "RH_Outer_streamlines.vtk"])
		print('Creating RH streamlines Done!')


	CSFDensdity_Path=os.path.join(RH_Directory,"RH_MID.CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	if CSFDensityExistenceTest==True :
		print('Computing RH EACSF Done')
	else :
		print('Computing RH EACSF  ')
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" ,"RH_MID.vtk", "--InputOuterStreamlines",  "RH_Outer_streamlines.vtk",\
			"--InputSegmentation", "CSF_Probability_Map.nrrd", "--InputMask", "RH_GM_Dilated.nrrd", "--OutputSurface", "RH_CSF_Density.vtk", "--VistitingMap",\
			"RH__Visitation.nrrd", "--SmoothingIter", '0', "--MaxVertexSmoothingDist", '0'])
		call_and_print([AddScalarstoPolyData, "--InputFile", "RH_GM.vtk", "--OutputFile", "RH_GM.vtk", "--ScalarsFile", "RH_MID.CSFDensity.txt", "--Scalars_Name", 'EACSF'])

	


parser = argparse.ArgumentParser(description='Calculates CSF Density')
parser.add_argument("--T1",type=str, help='T1 Image', default="@T1_IMAGE@")
parser.add_argument("--Tissu_Segmentation",type=str, help='Tissu Segmentation for Outer CSF Hull Creation', default="@Tissu_Segmentation@")
parser.add_argument("--CSF_Probability_Map",type=str, help='CSF Probality Map', default="@CSF_Probability_Map@")
parser.add_argument("--RH_MID_surface",type=str, help='Right Hemisphere MID Surface',default="@RH_MID_surface@")
parser.add_argument("--RH_GM_surface",type=str, help='Right Hemisphere GM Surface', default="@RH_GM_surface@")
parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
parser.add_argument('--CreateOuterImage', type=str, help='CreateOuterImage executable path', default='@CreateOuterImage_PATH@')
parser.add_argument('--CreateOuterSurface', type=str, help='CreateOuterSurface executable path', default='@CreateOuterSurface_PATH@')
parser.add_argument('--EditPolyData', type=str, help='EditPolyData executable path', default='@EditPolyData_PATH@')
parser.add_argument('--klaplace', type=str, help='klaplace executable path', default='@klaplace_PATH@')
parser.add_argument('--EstimateCortexStreamlinesDensity', type=str, help='EstimateCortexStreamlinesDensity executable path', default='@EstimateCortexStreamlinesDensity_PATH@')
parser.add_argument('--AddScalarstoPolyData', type=str, help='AddScalarstoPolyData executable path', default='@AddScalarstoPolyData_PATH@')
args = parser.parse_args()
main_loop(args)
