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
    #external process calling function with output and errors printing

	exe_path = args[0]
	print(" ".join(args))
	    
	completed_process = subprocess.run(args, capture_output=True, text=True)
	   
	status_code = completed_process.returncode
	out = completed_process.stdout
	err = completed_process.stderr
	check_returncode = completed_process.check_returncode()

	print("status code is:",status_code)
	if err!="":
		print("error: ",err)
	if out!="":
		print("output message :",out)
	if status_code != 0:
	   	print("return code:",check_returncode)

	

def main_loop(args):
	start = time.time()
	print ("Processing Left Side")
	process_LH(args) 
	end = time.time()
	print("time for LH:",end - start)

def process_LH(args):
	LH_Directory = os.path.join(args.Output_Directory,"LH_Directory") 
	isDir = os.path.isdir(LH_Directory)
	if isDir==False :
		os.mkdir(LH_Directory) 
		print("LH Directory created") 
	else :
		print ("Dirctory exist")

	T1 = os.path.join(LH_Directory, "T1.nrrd")
	Data_existence_test =os.path.isfile(T1) 
	if Data_existence_test==False:
		print("Copying Data")
		copyfile(args.T1, os.path.join(LH_Directory,"T1.nrrd"))
		copyfile(args.Seg, os.path.join(LH_Directory,"Seg.nrrd"))
		copyfile(args.CSF_Prop, os.path.join(LH_Directory,"CSF_Prop.nrrd"))
		copyfile(args.LH_MID_surface, os.path.join(LH_Directory,"LH_MID.vtk"))
		copyfile(args.LH_GM_surface, os.path.join(LH_Directory,"LH_GM.vtk"))
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

	##
	os.chdir(LH_Directory)
	Streamline_Path = os.path.join(LH_Directory,"LH_Outer_streamlines.vtk")
	StreamlinesExistenceTest = os.path.isfile(Streamline_Path)
	if StreamlinesExistenceTest ==True :
		print('LH streamline computation Done!')
	else:
		print('Creating Outer LH Convex Hull Surface')
		print('Creating LH Outer Image')
		call_and_print([CreateOuterImage,"--InputImg", "Seg.nrrd", "--OutputImg", "LH_GM_Dilated.nrrd", "--closingradius", '15', "--dilationradius", '2', "--Reverse", '0'])
		print('Creating LH Outer Surface')
		call_and_print([CreateOuterSurface,"--InputBinaryImg","LH_GM_Dilated.nrrd", "--OutputSurface","LH_GM_Outer_MC.vtk", "--NumberIterations",'1'])
		call_and_print([EditPolyData, "--InputSurface","LH_GM_Outer_MC.vtk", "--OutputSurface","LH_GM_Outer_MC.vtk", "--flipx", ' -1', "--flipy", ' -1', "--flipz", '1'])
		print('Creating Outer LH Convex Hull Surface Done!')


		print('Creating LH streamlines')
		print('CEstablishing Surface Correspondance')
		call_and_print([klaplace,'-dims','300',"LH_MID.vtk", "LH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"LH_Outer.corr"])

		print('CEstablishing Streamlines')
		call_and_print([klaplace, '-traceStream',"LH_Outer.corr_field.vts","LH_MID.vtk", "LH_GM_Outer_MC.vtk", "LH_Outer_streamlines.vtp", \
									"LH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"LH_Outer_streamlines.vtp", "LH_Outer_streamlines.vtk"])
		print('Creating LH streamlines Done!')


	# CSFDensdity_Path=os.path.join(LH_Directory,"LH_MID.CSFDensity.txt")
	# CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	# if CSFDensityExistenceTest==True :
	# 	print('Computing LH EACSF Done')
	# else :
	# 	print('Computing LH EACSF  ')
	# 	call_and_print([EstimateCortexStreamlinesDensity, "LH_MID.vtk", "LH_Outer_streamlines.vtk", "CSF_Prop.nrrd", "LH_GM_Dilated.nrrd", "LH_CSF_Density.vtk","LH__Visitation.nrrd",'0','0'])
	# 	call_and_print([AddScalarstoPolyData,"LH_GM.vtk", "LH_GM.vtk", "LH_MID.CSFDensity.txt", "EACSF"])



parser = argparse.ArgumentParser(description='Calculates CSF Density')
parser.add_argument("--T1",type=str, help='T1 Image', default="@T1_IMAGE@")
parser.add_argument("--Seg",type=str, help='Tissu Segmentation for Outer CSF Hull Creation', default="@Seg@")
parser.add_argument("--CSF_Prop",type=str, help='CSF Probality Map', default="@CSF_Prop@")
parser.add_argument("--LH_MID_surface",type=str, help='Left Hemisphere MID Surface', default="@LH_MID_surface@")
parser.add_argument("--LH_GM_surface",type=str, help='Left Hemisphere GM Surface', default="@LH_GM_surface@")
parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
parser.add_argument('--CreateOuterImage', type=str, help='CreateOuterImage executable path', default='@CreateOuterImage_PATH@')
parser.add_argument('--CreateOuterSurface', type=str, help='CreateOuterSurface executable path', default='@CreateOuterSurface_PATH@')
parser.add_argument('--EditPolyData', type=str, help='EditPolyData executable path', default='@EditPolyData_PATH@')
parser.add_argument('--klaplace', type=str, help='klaplace executable path', default='@klaplace_PATH@')
parser.add_argument('--EstimateCortexStreamlinesDensity', type=str, help='EstimateCortexStreamlinesDensity executable path', default='@EstimateCortexStreamlinesDensity_PATH@')
parser.add_argument('--AddScalarstoPolyData', type=str, help='AddScalarstoPolyData executable path', default='@AddScalarstoPolyData_PATH@')
args = parser.parse_args()

main_loop(args)
