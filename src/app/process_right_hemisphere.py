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

def Interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile):

	print("Interpolating ", flush=True)
	EACSFDensityMax = open(EACSFMaxValueFile, "r")
	EACSFDensityMin = open(EACSFMinValueFile, "r")
	EACSFDensityInterpolated = open(EACSFInterpolatedFile, "w")

	##### read and write the first lines
	number_of_points = EACSFDensityMax.readline()
	dimension = EACSFDensityMax.readline()
	Type = EACSFDensityMax.readline()

	EACSFDensityMin.readline()
	EACSFDensityMin.readline()
	EACSFDensityMin.readline()

	EACSFDensityInterpolated.write(number_of_points)
	EACSFDensityInterpolated.write(dimension)
	EACSFDensityInterpolated.write(Type)
	
	for line in EACSFDensityMax.readlines():
		valueMax = line
		valueMin = Decimal(EACSFDensityMin.readline())
		mu = (Decimal(valueMax) + valueMin)/2
		EACSFDensityInterpolated.write(str( mu ) + "\n" ) 
	EACSFDensityMax.close()
	EACSFDensityMin.close()
	EACSFDensityInterpolated.close()
	print("Interpolating done ", flush=True) 

def AbsoluteDifference(EACSFInterpolatedValueFile, EACSFFile, AbsoluteDifferenceFile):

	print("Computing absolute difference  ", flush=True)
	EACSFInterpolated = open(EACSFInterpolatedValueFile, "r")
	EACSF = open(EACSFFile, "r")
	AbsoluteDifference = open( AbsoluteDifferenceFile, "w")

	##### read and write the first lines
	number_of_points = EACSF.readline()
	dimension = EACSF.readline()
	Type = EACSF.readline()

	EACSFInterpolated.readline()
	EACSFInterpolated.readline()
	EACSFInterpolated.readline()

	AbsoluteDifference.write(number_of_points)
	AbsoluteDifference.write(dimension)
	AbsoluteDifference.write(Type)
	
	for line in EACSF.readlines():
		EACSFvalue = line
		EACSFInterpolatedvalue = float(EACSFInterpolated.readline())
		delta = EACSFInterpolatedvalue - float(EACSFvalue)
		AbsoluteDifference.write(str("%.4f" % delta) + "\n" ) 
	EACSF.close()
	EACSFInterpolated.close()
	AbsoluteDifference.close() 
	print("Computing done  ", flush=True)

def clean_up(Directory):

	print("Cleaning the right hemisphere output directory", flush=True)
	for i in os.listdir(Directory):
		if i.endswith('.vtp') or i.endswith('.vts'):
			os.remove(i)
	print("Cleaning the right hemisphere output directory done", flush=True)

def CSFDensity_Sum (CSF_Density_txtFile, CSF_Volume_txtFile):

	print("Computing the CSf density sum   ", flush=True)
	data = []
	with open(CSF_Density_txtFile) as inputfile:
		number_of_points = inputfile.readline()
		dimension = inputfile.readline()
		Type = inputfile.readline()
		for line in inputfile:
			fields = line.split()
			rowdata = map(float, fields)
			data.extend(rowdata)
	CSFDensity_Sum = sum(data)
	with open(CSF_Volume_txtFile, "a") as outputfile:
		outputfile.write("Sum of EACSF Density = " + str("%.0f" % CSFDensity_Sum))
	print("Computing the CSf density sum   ", flush=True)

def main_loop(args):

	start = time.time()
	print ("Processing Right Side", flush=True)

	ImageSizes = [@imagedimension@, int(@imagedimension@) + 20, int(@imagedimension@) - 20 ]
	DirectoriesNames = ["RH_Directory", "RH_InterpolationMaxValue_Directory", "RH_InterpolationMinValue_Directory" ]
	procs = []
	for i in range(len(ImageSizes)):
		process = Process(target=processing, args=(args, DirectoriesNames[i], str(ImageSizes[i]),))
		process.start()
		procs.append(process) 
	for process in procs:
		process.join() 

	EACSFFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_Directory", "RH_MID.CSFDensity.txt")
	EACSFMaxValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_InterpolationMaxValue_Directory", "RH_MID.CSFDensity.txt")
	EACSFMinValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_InterpolationMinValue_Directory", "RH_MID.CSFDensity.txt") 
	EACSFInterpolatedFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_Directory", "RH_MID.CSFDensityInterpolated.txt")	

	EACSFMagGradientMaxValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_InterpolationMaxValue_Directory", "RH_MID.CSFDensityMagGradient.txt")
	EACSFMagGradientMinValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_InterpolationMinValue_Directory", "RH_MID.CSFDensityMagGradient.txt") 
	EACSFMagGradientInterpolatedFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_Directory", "RH_MID.CSFDensityMagGradientInterpolated.txt")

	AbsoluteDifferenceFile = os.path.join( args.Output_Directory, "LocalEACSF", "RH_Directory", "RH_absolute_difference.txt" )

	Interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile)
	Interpolation(EACSFMagGradientMaxValueFile, EACSFMagGradientMinValueFile , EACSFMagGradientInterpolatedFile)
	AbsoluteDifference(EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile)


	### Interpolate the csf density
	os.chdir(os.path.join( args.Output_Directory, "LocalEACSF", "RH_Directory"))
	if(args.Interpolation) :
		call_and_print([args.AddScalarstoPolyData, "--InputFile", "RH_CSF_Density.vtk", "--OutputFile", "RH_CSF_Density.vtk", "--ScalarsFile", "RH_MID.CSFDensity.txt", "--Scalars_Name", 'CSFDensity'])
		call_and_print([args.AddScalarstoPolyData, "--InputFile", "RH_CSF_Density.vtk", "--OutputFile", "RH_CSF_Density.vtk", "--ScalarsFile", "RH_MID.CSFDensityMagGradient.txt", "--Scalars_Name", 'CSFDensityGradient'])
	######

	call_and_print([args.ComputeCSFVolume, "--VisitingMap", "RH__Visitation.nrrd", "--CSFProb", "CSF_Probability_Map.nrrd"])
	CSFDensity_Sum ("RH_MID.CSFDensity.txt", "RH_CSFVolume.txt")
	call_and_print([args.AddScalarstoPolyData, "--InputFile", "RH_GM.vtk", "--OutputFile", "RH_GM.vtk", "--ScalarsFile", "RH_absolute_difference.txt", "--Scalars_Name", 'Delta'])
	
	end = time.time()
	print("time for RH:",end - start, flush=True)

def processing(args, DirectoryName, ImageDimension):

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
		copyfile(args.RH_MID_surface, os.path.join(Directory,"RH_MID.vtk"))
		copyfile(args.RH_GM_surface, os.path.join(Directory,"RH_GM.vtk"))
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
	
	###
	os.chdir(Directory)
	Streamline_Path = os.path.join(Directory,"RH_Outer_streamlines.vtk")
	StreamlinesExistenceTest = os.path.isfile(Streamline_Path)
	if StreamlinesExistenceTest ==True :
		print('RH streamline computation Done!', flush=True)
	else:
		print('Creating Outer RH Convex Hull Surface', flush=True)
		print('Creating RH Outer Image', flush=True)
		call_and_print([CreateOuterImage,"--InputImg", "Tissu_Segmentation.nrrd", "--OutputImg","RH_GM_Dilated.nrrd", "--closingradius", "@closingradius@", "--dilationradius", "@dilationradius@", "--Reverse",'1'])
		print('Creating RH Outer Surface')
		call_and_print([CreateOuterSurface, "--InputBinaryImg", "RH_GM_Dilated.nrrd", "--OutputSurface", "RH_GM_Outer_MC.vtk", "--NumberIterations", "@NumberIterations@"])
		call_and_print([EditPolyData, "--InputSurface", "RH_GM_Outer_MC.vtk", "--OutputSurface", "RH_GM_Outer_MC.vtk", "--flipx", ' -1', "--flipy", ' -1', "--flipz", '1'])
		print('Creating Outer RH Convex Hull Surface Done!', flush=True)

		print('Creating RH streamlines', flush=True)
		print('CEstablishing Surface Correspondance', flush=True)
		call_and_print([klaplace,'-dims', ImageDimension,"RH_MID.vtk", "RH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"RH_Outer.corr"])

		print('CEstablishing Streamlines', flush=True)
		call_and_print([klaplace, '-traceStream',"RH_Outer.corr_field.vts","RH_MID.vtk", "RH_GM_Outer_MC.vtk", "RH_Outer_streamlines.vtp", \
									"RH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"RH_Outer_streamlines.vtp", "RH_Outer_streamlines.vtk"])
		print('Creating RH streamlines Done!', flush=True)


	CSFDensdity_Path=os.path.join(Directory,"RH_MID.CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	if CSFDensityExistenceTest==True :
		print('Computing RH EACSF Done', flush=True)
	else :
		print('Computing RH EACSF  ')
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" ,"RH_MID.vtk", "--InputOuterStreamlines",  "RH_Outer_streamlines.vtk",\
			"--InputSegmentation", "CSF_Probability_Map.nrrd", "--InputMask", "RH_GM_Dilated.nrrd", "--OutputSurface", "RH_CSF_Density.vtk", "--VisitingMap",\
			"RH__Visitation.nrrd"])
		if(args.Smooth) :
				call_and_print([HeatKernelSmoothing , "--InputSurface", "RH_CSF_Density.vtk", "--NumberIter", "@NumberIter@", "--sigma", "@Bandwith@", "--OutputSurface", "RH_CSF_Density.vtk"])
		call_and_print([AddScalarstoPolyData, "--InputFile", "RH_GM.vtk", "--OutputFile", "RH_GM.vtk", "--ScalarsFile", "RH_MID.CSFDensity.txt", "--Scalars_Name", 'EACSF'])
	if(args.Clean_up) :
	 	clean_up(Directory)



parser = argparse.ArgumentParser(description='EACSF Density Quantification')
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
parser.add_argument('--HeatKernelSmoothing', type=str, help='HeatKernelSmoothing executable path', default='@HeatKernelSmoothing_PATH@')
parser.add_argument('--ComputeCSFVolume', type=str, help='ComputeCSFVolume executable path', default='@ComputeCSFVolume_PATH@')
parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
parser.add_argument('--Clean_up', type=bool, help='Clean the output directory of intermediate outputs', default=@Clean@)
parser.add_argument('--Interpolation', type=bool, help='keep the interpolated CSF density', default=@Interpolation@)
args = parser.parse_args()
main_loop(args)