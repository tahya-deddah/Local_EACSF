#!/tools/Python/Python-3.8.0/bin/python3
##	by Tahya Deddah, Martin Styner, Juan Prieto
#########################################################################################################

import argparse
import time
import sys
import os
import shutil 
from shutil import copyfile
import subprocess
from subprocess import Popen
from multiprocessing import Process
import multiprocessing as mp
from multiprocessing import Pool
import threading
from decimal import *


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

def average(file):

	data = []
	with open(file) as f:
		number_of_points = f.readline()
		dimension = f.readline()
		Type = f.readline()
		for line in f:
			fields = line.split()
			rowdata = map(float, fields)
			data.extend(rowdata)
	average = sum(data)/len(data)
	return (average)

def avoid_double_counting(Directory):

	#Executables:
	CreateOuterImage = args.CreateOuterImage
	FitPlane = args.FitPlane

	os.chdir(Directory)
	call_and_print([CreateOuterImage,"--InputImg", "Tissu_Segmentation.nrrd", "--OutputImg", "RH_GM_Dilated.nrrd", "--closingradius", "@closingradius@",\
	 "--dilationradius", "@dilationradius@", "--Reverse", '1'])
	call_and_print([FitPlane,"--input1", "LH_GM_Dilated.nrrd", "--input2", "RH_GM_Dilated.nrrd", "--output1", "LH_GM_Dilated.nrrd", "--output2", "RH_GM_Dilated.nrrd"])
	os.remove("RH_GM_Dilated.nrrd")

def clean_up(Directory):

	print("Cleaning the directory", flush=True)
	for i in os.listdir(Directory):
		if i.endswith('.vtp') or i.endswith('.vts'):
			os.remove(i)
	print("Cleaning directory done", flush=True)
	
def interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile):

	print("interpolating ", flush=True)
	EACSFDensityMax = open(EACSFMaxValueFile, "r")
	EACSFDensityMin = open(EACSFMinValueFile, "r")
	EACSF = open(EACSFFile, "r")
	EACSFDensityInterpolated = open(EACSFInterpolatedFile, "w")
	AbsoluteDifference = open( AbsoluteDifferenceFile, "w")

	InputFiles = [EACSFDensityMax, EACSFDensityMin, EACSF ]
	for i in InputFiles :
		number_of_points = EACSFDensityMax.readline()
		dimension = EACSFDensityMax.readline()
		Type = EACSFDensityMax.readline()
	
	for line in EACSFDensityMax.readlines():
		valueMax = line
		valueMin = Decimal(EACSFDensityMin.readline())
		value = Decimal(EACSF.readline())
		mu = (Decimal(valueMax) + valueMin)/2
		delta = mu - Decimal(value)
		EACSFDensityInterpolated.write(str(mu) + "\n" )
		AbsoluteDifference.write(str("%.4f" % delta) + "\n" )  
	EACSFDensityMax.close()
	EACSFDensityMin.close()
	EACSF.close()
	EACSFDensityInterpolated.close() 
	AbsoluteDifference.close()
	print("Interpolation done ", flush=True)

def optimize_csfdensity (surface): ## optimize

	EACSFFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", "LH_" + surface + ".CSFDensity.txt")
	EACSFMaxValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_InterpolationMaxValue_Directory", "LH_" + surface + ".CSFDensity.txt")
	EACSFMinValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_InterpolationMinValue_Directory", "LH_" + surface + ".CSFDensity.txt") 
	EACSFInterpolatedFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", "LH_" + surface + ".CSFDensityInterpolated.txt")	
	AbsoluteDifferenceFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", "LH_absolute_difference.txt" )

	interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile)
	os.chdir(os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory"))
	call_and_print([args.AddScalarstoPolyData, "--InputFile", "LH_" + surface + "_CSF_Density.vtk", "--OutputFile", "LH_" + surface + "_CSF_Density.vtk",\
	"--ScalarsFile", "LH_absolute_difference.txt", "--Scalars_Name", 'Difference'])

	original_density_average = average(EACSFFile)
	optimal_density_average = average(EACSFInterpolatedFile)
	if(original_density_average > optimal_density_average or original_density_average == optimal_density_average):
		os.remove(InterpolatedFile)
	if(optimal_density_average > original_density_average):
		os.replace(EACSFInterpolatedFile, EACSFFile)		
	call_and_print([args.AddScalarstoPolyData, "--InputFile", "LH_" + surface + "_CSF_Density.vtk", "--OutputFile",  "LH_" + surface + "_CSF_Density.vtk",\
	 "--ScalarsFile", EACSFFile, "--Scalars_Name", 'CSFDensity'])	
	if(args.Clean_up):
		shutil.rmtree("../LH_InterpolationMaxValue_Directory")
		shutil.rmtree("../LH_InterpolationMinValue_Directory")

def main_loop(args):

	start = time.time()
	print ("Processing Left Side", flush=True)
	if(args.Use_MID_Surface):
		surface = "MID"
	if(args.Use_75P_Surface):
		surface = "75P"

	if(args.Interpolated):
		ImageSizes = [@imagedimension@, int(@imagedimension@) + int(@interpolationMargin@), int(@imagedimension@) - int(@interpolationMargin@) ]
		DirectoriesNames = ["LH_Directory", "LH_InterpolationMaxValue_Directory", "LH_InterpolationMinValue_Directory" ]
		pool = mp.Pool()
		for i in range(len(ImageSizes)):
			pool.apply_async(processing, args = (args, DirectoriesNames[i], surface, str(ImageSizes[i]),))
		pool.close()
		pool.join()
		optimize_csfdensity(surface)			
	if(args.NotInterpolated):
		processing(args, "LH_Directory", surface, str(@imagedimension@))

	os.chdir(os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory"))
	call_and_print([args.ComputeCSFVolume, "--VisitingMap", "LH__Visitation.nrrd", "--CSFProb", "CSF_Probability_Map.nrrd", "--CSFFile",EACSFFile])	
		
	end = time.time()
	print("time for LH:",end - start, flush=True)

def processing(args, DirectoryName, Surface, ImageDimension):

	Directory = os.path.join(args.Output_Directory, "LocalEACSF", DirectoryName) 
	isDir = os.path.isdir(Directory)
	if isDir==False :
		os.mkdir(Directory) 
		print("{} created".format(DirectoryName), flush=True) 
	else :
		print ("{} exist".format(DirectoryName), flush=True)

	T1 = os.path.join(Directory, "T1.nrrd")
	Data_existence_test =os.path.isfile(T1) 
	if Data_existence_test==False:
		print("Copying Data", flush=True)
		copyfile(args.T1, os.path.join(Directory,"T1.nrrd"))
		copyfile(args.Tissu_Segmentation, os.path.join(Directory,"Tissu_Segmentation.nrrd"))
		copyfile(args.CSF_Probability_Map, os.path.join(Directory,"CSF_Probability_Map.nrrd"))
		copyfile(args.LH_MID_surface, os.path.join(Directory,"LH_MID.vtk"))
		copyfile(args.LH_GM_surface, os.path.join(Directory,"LH_GM.vtk"))
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
	ComputeAverageMesh = args.ComputeAverageMesh
	
	os.chdir(Directory)	
	if(args.Use_75P_Surface):
		call_and_print([ComputeAverageMesh, "--inputFilename1", "LH_GM.vtk", "--inputFilename2", "LH_MID.vtk", "--outputFilename", "LH_75P.vtk"])
			
	Streamline_Path = os.path.join(Directory,"LH_Outer_streamlines.vtk")
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
		call_and_print([klaplace,'-dims', ImageDimension, "LH_" + Surface + ".vtk", "LH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"LH_Outer.corr"])
		print('CEstablishing Streamlines', flush=True)
		call_and_print([klaplace, '-traceStream',"LH_Outer.corr_field.vts", "LH_" + Surface + ".vtk", "LH_GM_Outer_MC.vtk", "LH_Outer_streamlines.vtp", \
									"LH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"LH_Outer_streamlines.vtp", "LH_Outer_streamlines.vtk"])
		print('Creating LH streamlines Done!', flush=True)

	avoid_double_counting(Directory)
	CSFDensdityTxtFile = os.path.join(Directory,"LH_" + Surface + ".CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdityTxtFile)
	if CSFDensityExistenceTest==True :
		print('Computing LH EACSF Done', flush=True)
	else :
		print('Computing LH EACSF  ', flush=True)
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" , "LH_" + Surface + ".vtk", "--InputOuterStreamlines",  "LH_Outer_streamlines.vtk",\
			"--InputSegmentation", "CSF_Probability_Map.nrrd", "--InputMask", "LH_GM_Dilated.nrrd", "--OutputSurface", "LH_" + Surface + "_CSF_Density.vtk", "--VisitingMap",\
			"LH__Visitation.nrrd"])
		if(args.Smooth) :
			call_and_print([HeatKernelSmoothing , "--InputSurface", "LH_" + Surface + "_CSF_Density.vtk", "--NumberIter", "@NumberIter@", "--sigma", "@Bandwith@", "--OutputSurface", "LH_" + Surface + "_CSF_Density.vtk"])
		call_and_print([AddScalarstoPolyData, "--InputFile", "LH_GM.vtk", "--OutputFile", "LH_GM.vtk", "--ScalarsFile", "LH_" + Surface + ".CSFDensity.txt", "--Scalars_Name", 'EACSF'])
	if(args.Clean_up) :
	 	clean_up(Directory)
			
parser = argparse.ArgumentParser(description='EACSF Density Quantification')
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
parser.add_argument('--ComputeCSFVolume', type=str, help='ComputeCSFVolume executable path', default='@ComputeCSFVolume_PATH@')
parser.add_argument('--ComputeAverageMesh', type=str, help='ComputeAverageMesh executable path', default='@ComputeAverageMesh_PATH@')
parser.add_argument('--FitPlane', type=str, help='FitPlane executable path', default='@FitPlane_PATH@')
parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
parser.add_argument('--Clean_up', type=bool, help='Clean the output directory of intermediate outputs', default=@Clean@)
parser.add_argument('--Interpolated', type=bool, help='Compute  the optimal CSF density ( Interpolated)', default=@Interpolated@)
parser.add_argument('--NotInterpolated', type=bool, help='Compute CSF density without optimisation (Interpolation)', default=@NotInterpolated@)
parser.add_argument('--Use_MID_Surface', type=bool, help='use the MID surface as the input surface', default=@Use_MID_Surface@)
parser.add_argument('--Use_75P_Surface', type=bool, help='use the 75P Surface as the input surface', default=@Use_75P_Surface@)
args = parser.parse_args()

main_loop(args)
