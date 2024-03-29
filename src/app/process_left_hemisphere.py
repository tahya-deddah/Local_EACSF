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
from main_script import call_and_print


### Compute the average of the values of a txt fule given as an input
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

### Clean up the outputs directory from intermediate outputs
def clean_up(Directory):
	print("Cleaning the directory", flush=True)
	for i in os.listdir(Directory):
		if i.endswith('.vtp') or i.endswith('.vts'):
			os.remove(i)
	print("Cleaning directory done", flush=True)

### Compute EACSF interpolated by doing a linear interpolation of EACSFMaxValueFile and  EACSFMinValueFile. Compute also the difference between the EACSF interpolated and the
## original EACSF (EACSFFile)
def interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile):

	print("interpolating ", flush=True)
	EACSFDensityMax = open(EACSFMaxValueFile, "r")
	EACSFDensityMin = open(EACSFMinValueFile, "r")
	EACSF = open(EACSFFile, "r")
	EACSFDensityInterpolated = open(EACSFInterpolatedFile, "w")
	AbsoluteDifference = open( AbsoluteDifferenceFile, "w")

	InputFiles = [EACSFDensityMax, EACSFDensityMin, EACSF ]
	OutputFiles = [EACSFDensityInterpolated,AbsoluteDifference]
	for i in InputFiles :
		number_of_points = i.readline()
		dimension = i.readline()
		Type = i.readline()
	for i in OutputFiles :
		i.write(number_of_points)
		i.write(dimension)
		i.write(Type)

	for line in EACSFDensityMax.readlines():
		valueMax = line
		valueMin = Decimal(EACSFDensityMin.readline())
		value = Decimal(EACSF.readline())
		mu = (Decimal(valueMax) + valueMin)/2
		delta = mu -value
		EACSFDensityInterpolated.write(str(mu) + "\n" )
		AbsoluteDifference.write(str("%.4f" % delta) + "\n" )  
	print("Interpolation done ", flush=True)

###  Compare the average of origial EACSF and the average of  the interpolated EACSF and choose the one with higher average as the final one  
def optimize_csfdensity (surface): ## optimize

	EACSFFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density.txt")
	EACSFMaxValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_InterpolationMaxValue_Directory", args.Label + "_LH_" + surface + "_CSF_Density.txt")
	EACSFMinValueFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_InterpolationMinValue_Directory", args.Label + "_LH_" + surface + "_CSF_Density.txt") 
	EACSFInterpolatedFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Interpolated.txt")
	EACSFFinalFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt")		
	AbsoluteDifferenceFile = os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Original_Interpolated_Absolute_Difference.txt" )

	interpolation(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile)
	os.chdir(os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory"))

	### add scalars to csfdensity.vtk
	call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk",\
	"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density.txt", "--Scalars_Name", 'CSF_Density_Original'])
	call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk",\
	"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_Interpolated.txt", "--Scalars_Name", 'CSF_Density_Interpolated'])

	### add scalars to csfdensity_inflated
	if(args.LH_Inflating_Template != ""):
		copyfile(args.LH_Inflating_Template, args.Label + "_LH_" + surface + "_Inflated.vtk")
		call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_Inflated.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_Inflated.vtk",\
		"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_Interpolated.txt", "--Scalars_Name", 'CSF_Density_Interpolated'])
		call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_Inflated.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_Inflated.vtk",\
		"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density.txt", "--Scalars_Name", 'CSF_Density_Original'])

	original_density_average = average(EACSFFile)
	optimal_density_average = average(EACSFInterpolatedFile)
	
	if(original_density_average > optimal_density_average or original_density_average == optimal_density_average):
		#copyfile(EACSFInterpolatedFile, EACSFFinalFile)
		shutil.copy(EACSFInterpolatedFile, EACSFFinalFile)

	if(optimal_density_average > original_density_average):
		#copyfile(EACSFFile, EACSFFinalFile)
		shutil.copy(EACSFFile, EACSFFinalFile)

	os.rename(EACSFFile, os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Original.txt"))	
	call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk",\
		"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt", "--Scalars_Name", 'CSF_Density_Final'])
	
	if(args.Clean_up):
		shutil.rmtree("../LH_InterpolationMaxValue_Directory")
		shutil.rmtree("../LH_InterpolationMinValue_Directory")

def main_loop(args):

	start = time.time()	
	if(args.Use_75P_Surface):
		surface = "75P"
	else :
		surface = "MID"

	CSFDensity_Path = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt")
	if (os.path.isfile(CSFDensity_Path)):
		print('Compute local EACSF density for left hemisphere done!', flush=True)
	else:
		print ("Processing Left Side", flush=True)
		### if interpolation
		if(args.Interpolated):
			ImageSizes = [@imagedimension@, int(@imagedimension@) + int(@interpolationMargin@), int(@imagedimension@) - int(@interpolationMargin@) ]
			DirectoriesNames = ["LH_Directory", "LH_InterpolationMaxValue_Directory", "LH_InterpolationMinValue_Directory" ]
			pool = mp.Pool()
			for i in range(len(ImageSizes)):
				pool.apply_async(processing, args = (args, DirectoriesNames[i], surface, str(ImageSizes[i]),))
			pool.close()
			pool.join()
			optimize_csfdensity(surface)
		### if not interplation			
		if(args.NotInterpolated):
			processing(args, "LH_Directory", surface, str(@imagedimension@))
			os.rename(os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label +"_LH_" + surface + "_CSF_Density.txt"),\
				os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt"))

		######
		os.chdir(os.path.join( args.Output_Directory, "LocalEACSF", "LH_Directory"))
		##### smooth the csf density
		if(args.Smooth) :
			call_and_print([args.HeatKernelSmoothing , "--InputSurface", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--NumberIter", "@NumberIter@", "--sigma", "@Bandwith@",\
			"--OutputSurface", args.Label +"_LH_" + surface + "_CSF_Density.vtk"])

		##### add scalars to inflated surfaces
		if(args.LH_Inflating_Template != ""):
			copyfile(args.LH_Inflating_Template, args.Label + "_LH_" + surface + "_Inflated.vtk")
			call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label +"_LH_" + surface + "_Inflated.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_Inflated.vtk",\
			"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt", "--Scalars_Name", 'CSF_Density_Final'])
			if(args.Smooth) :
				call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_Inflated.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_Inflated.vtk",\
				"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_Final_Smoothed.txt", "--Scalars_Name", 'CSF_Density_Final_Smoothed'])

		##### Compute regional mean and sum of EA-CSF
		if(args.Compute_regional_CSF_density):
			if(args.Smooth) :
				call_and_print([args.ROImean, "--InputMeasurement", args.Label + "_LH_" + surface + "_CSF_Density_Final_Smoothed.txt", "--AtlasSurfaceLabeling", args.Left_Atlas_Surface,\
				"--OutputFileName1", args.Label + "_LH_" + surface + "_CSF_Density_regional_mean.txt", "--OutputFileName2", args.Label + "_LH_" + surface + "_CSF_Density_regional_sum.txt"])
			else :
				call_and_print([args.ROImean, "--InputMeasurement", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt", "--AtlasSurfaceLabeling", args.Left_Atlas_Surface,\
				"--OutputFileName1", args.Label + "_LH_" + surface + "_CSF_Density_regional_mean.txt","--OutputFileName2", args.Label + "_LH_" + surface + "_CSF_Density_regional_sum.txt"])	
			
			call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk",\
			"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_regional_mean.txt", "--Scalars_Name", 'CSF_Density_Regional_Mean'])
			call_and_print([args.AddScalarstoPolyData, "--InputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk", "--OutputFile", args.Label + "_LH_" + surface + "_CSF_Density.vtk",\
			"--ScalarsFile", args.Label + "_LH_" + surface + "_CSF_Density_regional_sum.txt", "--Scalars_Name", 'CSF_Density_Regional_Sum'])

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

	TissuSeg = os.path.join(Directory, args.Label + "_Tissu_Segmentation.nrrd")
	Data_existence_test =os.path.isfile(TissuSeg) 
	if Data_existence_test==False:
		print("Copying Data", flush=True)
		copyfile(args.Tissu_Segmentation, os.path.join(Directory, args.Label + "_Tissu_Segmentation.nrrd"))
		copyfile(args.CSF_Probability_Map, os.path.join(Directory, args.Label +  "_CSF_Probability_Map.nrrd"))
		copyfile(args.LH_MID_surface, os.path.join(Directory, args.Label + "_LH_MID.vtk"))
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
	FitPlane = args.FitPlane
	
	os.chdir(Directory)	
	
	############ Generation of streamlines 		
	Streamline_Path = os.path.join(Directory, args.Label + "_LH_Outer_streamlines.vtk")
	StreamlinesExistenceTest = os.path.isfile(Streamline_Path)
	if StreamlinesExistenceTest ==True :
		print('LH streamline computation Done!', flush=True)
	else:
		print('Creating Outer LH Convex Hull Surface', flush=True)
		print('Creating LH Outer Image')
		call_and_print([CreateOuterImage,"--InputImg", args.Label + "_Tissu_Segmentation.nrrd", "--OutputImg", args.Label + "_LH_GM_Dilated.nrrd", "--closingradius", "@closingradius@", "--dilationradius", "@dilationradius@", "--Reverse", '0'])
		print('Creating LH Outer Surface', flush=True)
		call_and_print([CreateOuterSurface,"--InputBinaryImg", args.Label + "_LH_GM_Dilated.nrrd", "--OutputSurface", args.Label +"_LH_GM_Outer_MC.vtk", "--NumberIterations", "@NumberIterations@"])
		call_and_print([EditPolyData, "--InputSurface",args.Label + "_LH_GM_Outer_MC.vtk", "--OutputSurface", args.Label + "_LH_GM_Outer_MC.vtk", "--flipx", ' -1', "--flipy", ' -1', "--flipz", '1'])
		print('Creating Outer LH Convex Hull Surface Done!', flush=True)

		if(args.Use_75P_Surface):
			copyfile(args.LH_GM_surface, os.path.join(Directory, args.Label + "_LH_GM.vtk"))
			print('Creating inner surface:', flush=True)
			call_and_print([ComputeAverageMesh, "--inputFilename1", args.Label + "_LH_MID.vtk", "--inputFilename2", args.Label + "_LH_GM.vtk", "--outputFilename", args.Label +"_LH_75P.vtk"])
	
		print('Creating LH streamlines', flush=True)
		print('CEstablishing Surface Correspondance', flush=True)
		
		call_and_print([klaplace,'-dims', ImageDimension, args.Label + "_LH_" + Surface + ".vtk", args.Label + "_LH_GM_Outer_MC.vtk",'-surfaceCorrespondence',args.Label +"_LH_Outer.corr"])
		print('CEstablishing Streamlines', flush=True)
		call_and_print([klaplace, '-traceStream',args.Label +"_LH_Outer.corr_field.vts", args.Label +"_LH_" + Surface + ".vtk", args.Label + "_LH_GM_Outer_MC.vtk", args.Label + "_LH_Outer_streamlines.vtp", \
									args.Label + "_LH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv', args.Label + "_LH_Outer_streamlines.vtp", args.Label + "_LH_Outer_streamlines.vtk"])
		print('Creating LH streamlines Done!', flush=True)

	############ Avoid double counting by fitting a mid sagital plan
	OuterImageAfterFittingPlane = os.path.join(Directory, args.Label + "_LH_New_GM_Dilated.nrrd")  
	OuterImageAfterFittingPlaneExistenceTest = os.path.isfile(OuterImageAfterFittingPlane)
	if OuterImageAfterFittingPlaneExistenceTest==True :
		print('Fitting a plane to avoid double counting done ', flush=True)
	else :
		print('Fitting a plane to avoid EACSF double counting ', flush=True)
		
		call_and_print([CreateOuterImage,"--InputImg", args.Label + "_Tissu_Segmentation.nrrd", "--OutputImg", args.Label + "_RH_GM_Dilated.nrrd", \
			"--closingradius", "@closingradius@","--dilationradius", "@dilationradius@", "--Reverse", '1'])
		call_and_print([FitPlane,"--input1", args.Label + "_LH_GM_Dilated.nrrd", "--input2", args.Label + "_RH_GM_Dilated.nrrd", "--output1", \
		args.Label + "_LH_New_GM_Dilated.nrrd", "--output2", args.Label + "_RH_New_GM_Dilated.nrrd"])
		os.remove(args.Label + "_RH_GM_Dilated.nrrd")
		os.remove(args.Label + "_RH_New_GM_Dilated.nrrd")

	############ Computation of EACSF in the streamlines
	CSFDensdityTxtFile = os.path.join(Directory, args.Label +"_LH_" + Surface + "_CSF_Density.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdityTxtFile)
	if CSFDensityExistenceTest==True :
		print('Computing LH EACSF Done', flush=True)
	else :
		print('Computing LH EACSF  ', flush=True)
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" , args.Label + "_LH_" + Surface + ".vtk", "--InputOuterStreamlines", args.Label + "_LH_Outer_streamlines.vtk",\
			"--InputSegmentation", args.Label + "_CSF_Probability_Map.nrrd", "--InputMask", args.Label + "_LH_New_GM_Dilated.nrrd", "--OutputSurface", args.Label + "_LH_" + Surface + "_CSF_Density.vtk", "--VisitingMap",\
			args.Label + "_LH_Visitation.nrrd", "--VisitingLength", args.Label + "_LH_Visitation_Length.nrrd" ])
	if(args.Clean_up) :
	 	clean_up(Directory)
			
parser = argparse.ArgumentParser(description='EACSF Density Quantification for the left hemisphere')
parser.add_argument("--Tissu_Segmentation",type=str, help='Tissu Segmentation for Outer CSF Hull Creation', default="@Tissu_Segmentation@")
parser.add_argument("--CSF_Probability_Map",type=str, help='CSF Probality Map', default="@CSF_Probability_Map@")
parser.add_argument("--LH_MID_surface",type=str, help='Left Hemisphere MID Surface', default="@LH_MID_surface@")
parser.add_argument("--LH_GM_surface",type=str, help='Left Hemisphere GM Surface', default="@LH_GM_surface@")
parser.add_argument("--LH_Inflating_Template",type=str, help='Left Hemisphere Inflated Surface', default="@LH_Inflating_Template@")
parser.add_argument("--Left_Atlas_Surface",type=str, help='Left Atlas Surface Labeling', default="@Left_Atlas_Surface@")
parser.add_argument("--Label",type=str, help='Label of the case , ID for example', default="@Label@")
parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
parser.add_argument('--CreateOuterImage', type=str, help='CreateOuterImage executable path', default='@CreateOuterImage_PATH@')
parser.add_argument('--CreateOuterSurface', type=str, help='CreateOuterSurface executable path', default='@CreateOuterSurface_PATH@')
parser.add_argument('--EditPolyData', type=str, help='EditPolyData executable path', default='@EditPolyData_PATH@')
parser.add_argument('--klaplace', type=str, help='klaplace executable path', default='@klaplace_PATH@')
parser.add_argument('--EstimateCortexStreamlinesDensity', type=str, help='EstimateCortexStreamlinesDensity executable path', default='@EstimateCortexStreamlinesDensity_PATH@')
parser.add_argument('--AddScalarstoPolyData', type=str, help='AddScalarstoPolyData executable path', default='@AddScalarstoPolyData_PATH@')
parser.add_argument('--HeatKernelSmoothing', type=str, help='HeatKernelSmoothing executable path', default='@HeatKernelSmoothing_PATH@')
parser.add_argument('--ComputeAverageMesh', type=str, help='ComputeAverageMesh executable path', default='@ComputeAverageMesh_PATH@')
parser.add_argument('--FitPlane', type=str, help='FitPlane executable path', default='@FitPlane_PATH@')
parser.add_argument('--ROImean', type=str, help='ROImean executable path', default='@ROImean_PATH@')
parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
parser.add_argument('--Clean_up', type=bool, help='Clean the output directory of intermediate outputs', default=@Clean@)
parser.add_argument('--Interpolated', type=bool, help='Compute  the optimal CSF density ( Interpolated)', default=@Interpolated@)
parser.add_argument('--NotInterpolated', type=bool, help='Compute CSF density without optimisation (Interpolation)', default=@NotInterpolated@)
parser.add_argument('--Use_75P_Surface', type=bool, help='use the 75P Surface as the input surface', default=@Use_75P_Surface@)
parser.add_argument('--Compute_regional_CSF_density', type=bool, help='Compute regional CSF density', default=@Compute_regional_CSF_density@)
args = parser.parse_args()

main_loop(args)
