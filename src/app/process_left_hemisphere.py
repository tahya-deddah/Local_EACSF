import argparse
import time
import sys
import os
import shutil 
from shutil import copyfile
import subprocess
from subprocess import Popen
from multiprocessing import Process


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

def ComputeInterpolatedCSFDensity(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile):

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
		EACSFDensityInterpolated.write(str(mu) + "\n" ) 
	EACSFDensityMax.close()
	EACSFDensityMin.close()
	EACSFDensityInterpolated.close() 

def AbsoluteDifference(EACSFInterpolatedValueFile, EACSFFile, AbsoluteDifferenceFile):

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

def clean_up(LH_Directory):

	print("Cleaning the left hemisphere output directory", flush=True)
	for i in os.listdir(LH_Directory):
		if i.endswith('.vtp') or i.endswith('.vts'):
			os.remove(i)
	print("Cleaning the left hemisphere output directory done", flush=True)

def CSFDensity_Sum (CSF_Density_txtFile, CSF_Volume_txtFile):
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
		outputfile.write("Sum of EACSF Density = " + str("%.1f" % CSFDensity_Sum))

def main_loop(args):

	start = time.time()
	print ("Processing Left Side", flush=True)

	LH_Directory = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory") 
	isDir = os.path.isdir(LH_Directory)
	if isDir==False :
		os.mkdir(LH_Directory) 
		print("LH Directory created", flush=True) 
	else :
		print ("Dirctory exist", flush=True)

	CSFDensdity_Path=os.path.join(LH_Directory,"LH_MID.CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	if CSFDensityExistenceTest==True :
		print('Computing LH EACSF Done', flush=True)
	else :
		InterpolationMaxValue_Directory = os.path.join(LH_Directory, "InterpolationMaxValue") 
		InterpolationMinValue_Directory = os.path.join(LH_Directory, "InterpolationMinValue") 
		if os.path.isdir(InterpolationMaxValue_Directory)==False :
			os.mkdir(InterpolationMaxValue_Directory)
		if os.path.isdir(InterpolationMinValue_Directory)==False :
			os.mkdir(InterpolationMinValue_Directory)

		ImageSize = @imagedimension@
		ImageSizeMax = int(ImageSize) + 20
		ImageSizeMin = int(ImageSize) - 20
		procs = []
		ProcessingWithoutInterpolation = Process(target=processing, args=(args, LH_Directory, str(ImageSize),))
		ProcessingWithoutInterpolation.start()   
		procs.append(ProcessingWithoutInterpolation)               
		InterpolationMaxValueProcessing = Process(target=processing, args=(args,InterpolationMaxValue_Directory, str(ImageSizeMax),))
		InterpolationMaxValueProcessing.start()
		procs.append(InterpolationMaxValueProcessing) 
		InterpolationMinValueProcessing = Process(target=processing, args=(args, InterpolationMinValue_Directory, str(ImageSizeMin),))
		InterpolationMinValueProcessing.start()
		procs.append(InterpolationMinValueProcessing)
		for process in procs:
			process.join() 
		EACSFMaxValueFile = os.path.join( InterpolationMaxValue_Directory, "LH_MID.CSFDensity.txt")
		EACSFMinValueFile = os.path.join( InterpolationMinValue_Directory, "LH_MID.CSFDensity.txt") 
		EACSFInterpolatedFile = os.path.join( LH_Directory, "LH_MID.CSFDensityInterpolated.txt")
		EACSFFile = os.path.join( LH_Directory, "LH_MID.CSFDensity.txt")
		AbsoluteDifferenceFile = os.path.join( LH_Directory, "LH_absolute_difference.txt" )
		ComputeInterpolatedCSFDensity(EACSFMaxValueFile, EACSFMinValueFile , EACSFInterpolatedFile)
		AbsoluteDifference(EACSFInterpolatedFile, EACSFFile, AbsoluteDifferenceFile)
		shutil.rmtree(InterpolationMaxValue_Directory)
		shutil.rmtree(InterpolationMinValue_Directory)
		
		AddScalarstoPolyData = args.AddScalarstoPolyData
		HeatKernelSmoothing = args.HeatKernelSmoothing
		ComputeCSFVolume = args.ComputeCSFVolume
		os.chdir(LH_Directory)
		if(args.Interpolation) :
			if os.path.isfile(EACSFFile)==True :
				os.remove(EACSFFile)
			os.rename(EACSFInterpolatedFile, EACSFFile)
			call_and_print([AddScalarstoPolyData, "--InputFile", "LH_CSF_Density.vtk", "--OutputFile", "LH_CSF_Density.vtk", "--ScalarsFile", \
			"LH_MID.CSFDensity.txt", "--Scalars_Name", 'CSFDensity'])
		if(args.Smooth) :
				call_and_print([HeatKernelSmoothing , "--InputSurface", "LH_CSF_Density.vtk", "--NumberIter", "@NumberIter@", "--sigma", "@Bandwith@",\
				"--OutputSurface", "LH_CSF_Density.vtk"])
		if(args.Clean_up) :
		 	clean_up(LH_Directory)
		call_and_print([ComputeCSFVolume, "--VisitingMap", "LH__Visitation.nrrd", "--CSFProb", "CSF_Probability_Map.nrrd"])
		CSFDensity_Sum ("LH_MID.CSFDensity.txt", "LH_CSFVolume.txt")
		call_and_print([AddScalarstoPolyData, "--InputFile", "LH_GM.vtk", "--OutputFile", "LH_GM.vtk", "--ScalarsFile", "LH_absolute_difference.txt", "--Scalars_Name", 'Delta'])
	end = time.time()
	print("time for LH:",end - start, flush=True)

def processing(args, Directory, ImageDimension):

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
	
	os.chdir(Directory)
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
		call_and_print([klaplace,'-dims', ImageDimension,"LH_MID.vtk", "LH_GM_Outer_MC.vtk",'-surfaceCorrespondence',"LH_Outer.corr"])
		print('CEstablishing Streamlines', flush=True)
		call_and_print([klaplace, '-traceStream',"LH_Outer.corr_field.vts","LH_MID.vtk", "LH_GM_Outer_MC.vtk", "LH_Outer_streamlines.vtp", \
									"LH_Outer_points.vtp",'-traceDirection','forward'])
		call_and_print([klaplace, '-conv',"LH_Outer_streamlines.vtp", "LH_Outer_streamlines.vtk"])
		print('Creating LH streamlines Done!', flush=True)

	CSFDensdity_Path=os.path.join(Directory,"LH_MID.CSFDensity.txt")
	CSFDensityExistenceTest = os.path.isfile(CSFDensdity_Path)
	if CSFDensityExistenceTest==True :
		print('Computing LH EACSF Done', flush=True)
	else :
		print('Computing LH EACSF  ', flush=True)
		call_and_print([EstimateCortexStreamlinesDensity, "--InputSurface" ,"LH_MID.vtk", "--InputOuterStreamlines",  "LH_Outer_streamlines.vtk",\
			"--InputSegmentation", "CSF_Probability_Map.nrrd", "--InputMask", "LH_GM_Dilated.nrrd", "--OutputSurface", "LH_CSF_Density.vtk", "--VisitingMap",\
			"LH__Visitation.nrrd"])
		
	
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
parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
parser.add_argument('--Clean_up', type=bool, help='Clean the output directory of intermediate outputs', default=@Clean@)
parser.add_argument('--Interpolation', type=bool, help='keep the interpolated CSF density', default=@Interpolation@)
args = parser.parse_args()

main_loop(args)
