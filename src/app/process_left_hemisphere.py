import argparse
import time
import sys
import os
import shutil 
from shutil import copyfile
import subprocess
from subprocess import Popen


def call_and_print(args):
    #external process calling function with output and errors printing

	exe_path = args[0]
	print(" ".join(args), flush=True)
	    
	completed_process = subprocess.run(args, capture_output=True, text=True)
	   
	status_code = completed_process.returncode
	out = completed_process.stdout
	err = completed_process.stderr
	check_returncode = completed_process.check_returncode()

	print("status code is:",status_code, flush=True)
	if err!="":
		print(err, flush=True)
	if out!="":
		print(out, flush=True)
	if status_code != 0:
	   	print("return code:",check_returncode, flush=True)

def clean_up(LH_Directory):

	print("Cleaning the left hemisphere output directory", flush=True)
	for i in os.listdir(LH_Directory):
		if i.endswith('.vtp') or i.endswith('.vts'):
			os.remove(i)
	print("Cleaning the left hemisphere output directory done", flush=True)


	

def main_loop(args):
	start = time.time()
	print ("Processing Left Side", flush=True)
	process_LH(args) 
	end = time.time()
	print("time for LH:",end - start, flush=True)

def process_LH(args):
	LH_Directory = os.path.join(args.Output_Directory,"LH_Directory") 
	isDir = os.path.isdir(LH_Directory)
	if isDir==False :
		os.mkdir(LH_Directory) 
		print("LH Directory created", flush=True) 
	else :
		print ("Dirctory exist", flush=True)

	T1 = os.path.join(LH_Directory, "T1.nrrd")
	Data_existence_test =os.path.isfile(T1) 
	if Data_existence_test==False:
		print("Copying Data", flush=True)
		copyfile(args.T1, os.path.join(LH_Directory,"T1.nrrd"))
		copyfile(args.Tissu_Segmentation, os.path.join(LH_Directory,"Tissu_Segmentation.nrrd"))
		copyfile(args.CSF_Probability_Map, os.path.join(LH_Directory,"CSF_Probability_Map.nrrd"))
		copyfile(args.LH_MID_surface, os.path.join(LH_Directory,"LH_MID.vtk"))
		copyfile(args.LH_GM_surface, os.path.join(LH_Directory,"LH_GM.vtk"))
		print("Copying Done", flush=True)
	else :
		print("Data Exists", flush=True)	


	#Executables:
	CreateOuterImage = args.CreateOuterImage
	CreateOuterSurface = args.CreateOuterSurface
	EditPolyData = args.EditPolyData
	klaplace = args.klaplace
	EstimateCortexStreamlinesDensity = args.EstimateCortexStreamlinesDensity
	AddScalarstoPolyData = args.AddScalarstoPolyData
	HeatKernelSmoothing = args.HeatKernelSmoothing

	##
	os.chdir(LH_Directory)
	Streamline_Path = os.path.join(LH_Directory,"LH_Outer_streamlines.vtk")
	StreamlinesExistenceTest = os.path.isfile(Streamline_Path)
	if StreamlinesExistenceTest ==True :
		print('LH streamline computation Done!', flush=True)
	else:
		print('Creating Outer LH Convex Hull Surface', flush=True)
		print('Creating LH Outer Image')
		call_and_print([CreateOuterImage,"--InputImg", "Tissu_Segmentation.nrrd", "--OutputImg", "LH_GM_Dilated.nrrd", "--closingradius", "@closingradius@", "--dilationradius", "@dilationradius@", "--Reverse", '0'])
		print('Creating LH Outer Surface', flush=True)
		call_and_print([CreateOuterSurface,"--InputBinaryImg","LH_GM_Dilated.nrrd", "--OutputSurface","LH_GM_Outer_MC.vtk", "--NumberIterations", "@NumberIterations@"])
		call_and_print([EditPolyData, "--InputSurface","LH_GM_Outer_MC.vtk", "--OutputSurface","LH_GM_Outer_MC.vtk", "--flipx", ' -1', "--flipy", ' -1', "--flipz", '1'])
		print('Creating Outer LH Convex Hull Surface Done!', flush=True)
	
		print('Creating LH streamlines', flush=True)
		print('CEstablishing Surface Correspondance', flush=True)
		call_and_print([klaplace,'-dims', "@imagedimension@","LH_MID.vtk", "LH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"LH_Outer.corr"])

		print('CEstablishing Streamlines', flush=True)
		call_and_print([klaplace, '-traceStream',"LH_Outer.corr_field.vts","LH_MID.vtk", "LH_GM_Outer_MC.vtk", "LH_Outer_streamlines.vtp", \
									"LH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"LH_Outer_streamlines.vtp", "LH_Outer_streamlines.vtk"])
		print('Creating LH streamlines Done!', flush=True)


	CSFDensdity_Path=os.path.join(LH_Directory,"LH_MID.CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	if CSFDensityExistenceTest==True :
		print('Computing LH EACSF Done', flush=True)
	else :
		print('Computing LH EACSF  ', flush=True)
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" ,"LH_MID.vtk", "--InputOuterStreamlines",  "LH_Outer_streamlines.vtk",\
			"--InputSegmentation", "CSF_Probability_Map.nrrd", "--InputMask", "LH_GM_Dilated.nrrd", "--OutputSurface", "LH_CSF_Density.vtk", "--VistitingMap",\
			"LH__Visitation.nrrd"])
		if(args.Smooth) :
			call_and_print([HeatKernelSmoothing , "--InputSurface", "LH_CSF_Density.vtk", "--NumberIter", "@NumberIter@", "--sigma", "@Bandwith@", "--OutputSurface", "LH_CSF_Density.vtk"])
			call_and_print([AddScalarstoPolyData, "--InputFile", "LH_GM.vtk", "--OutputFile", "LH_GM.vtk", "--ScalarsFile", "LH_SmoothedCSFDensity.txt", "--Scalars_Name", 'EACSF'])
		else :
			call_and_print([AddScalarstoPolyData, "--InputFile", "LH_GM.vtk", "--OutputFile", "LH_GM.vtk", "--ScalarsFile", "LH_MID.CSFDensity.txt", "--Scalars_Name", 'EACSF'])
	if(args.Clean_up) :
	 	clean_up(LH_Directory)


parser = argparse.ArgumentParser(description='Calculates CSF Density')
parser.add_argument("--T1",type=str, help='T1 Image', default="@T1_IMAGE@")
parser.add_argument("--Tissu_Segmentation",type=str, help='Tissu Segmentation for Outer CSF Hull Creation', default="@Tissu_Segmentation@")
parser.add_argument("--CSF_Probability_Map",type=str, help='CSF Probality Map', default="@CSF_Probability_Map@")
parser.add_argument("--LH_MID_surface",type=str, help='Left Hemisphere MID Surface', default="@LH_MID_surface@")
parser.add_argument("--LH_GM_surface",type=str, help='Left Hemisphere GM Surface', default="@LH_GM_surface@")
parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
parser.add_argument('--CreateOuterImage', type=str, help='CreateOuterImage executable path', default='@CreateOuterImage_PATH@')
parser.add_argument('--CreateOuterSurface', type=str, help='CreateOuterSurface executable path', default='@CreateOuterSurface_PATH@')
parser.add_argument('--EditPolyData', type=str, help='EditPolyData executable path', default='@EditPolyData_PATH@')
parser.add_argument('--klaplace', type=str, help='klaplace executable path', default='@klaplace_PATH@')
parser.add_argument('--EstimateCortexStreamlinesDensity', type=str, help='EstimateCortexStreamlinesDensity executable path', default='@EstimateCortexStreamlinesDensity_PATH@')
parser.add_argument('--AddScalarstoPolyData', type=str, help='AddScalarstoPolyData executable path', default='@AddScalarstoPolyData_PATH@')
parser.add_argument('--HeatKernelSmoothing', type=str, help='HeatKernelSmoothing executable path', default='@HeatKernelSmoothing_PATH@')
parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
parser.add_argument('--Clean_up', type=bool, help='Clean the output directory of intermediate outputs', default=@Clean@)
args = parser.parse_args()

main_loop(args)
