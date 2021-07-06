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
import os.path
from os import path
import csv


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


def main(args):
		
	if(args.Use_75P_Surface):
		surface = "75P"
	else :
		surface = "MID"

	#Executables
	python = args.python3
	ComputeCSFVolume = args.ComputeCSFVolume

	#Output Path
	OUT_PATH = os.path.join(args.Output_Directory,"LocalEACSF") 	
	scripts_prefix = os.path.join(OUT_PATH, "PythonScripts")
	process_left_hemisphere = os.path.join(scripts_prefix,"process_left_hemisphere.py")
	process_right_hemisphere = os.path.join(scripts_prefix,"process_right_hemisphere.py")
	LH_CSFDensity_Path = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density_Final.txt")
	RH_CSFDensity_Path = os.path.join(args.Output_Directory, "LocalEACSF", "RH_Directory", args.Label + "_RH_" + surface + "_CSF_Density_Final.txt")
	####
	if(path.exists(os.path.join(OUT_PATH, args.Label + "_CSFVolume.txt"))):
		print('Compute Local EACSF density already done for ID {} is already done'.format(args.Label) , flush=True)
	else :
		Process_Left_Side = subprocess.call([python, process_left_hemisphere])
		Process_Right_Side = subprocess.call([python, process_right_hemisphere])
	
		if (os.path.isfile(LH_CSFDensity_Path) and os.path.isfile(RH_CSFDensity_Path)):
			os.chdir(OUT_PATH)
			if(args.Smooth) :
				call_and_print([ComputeCSFVolume, "--VisitingMap", os.path.join("LH_Directory", args.Label + "_LH_Visitation.nrrd"), "--CSFProb",\
				os.path.join("LH_Directory", args.Label + "_CSF_Probability_Map.nrrd"),"--CSFFile", os.path.join("LH_Directory", args.Label\
				+ "_LH_" + surface + "_CSF_Density_Final_Smoothed.txt") , "--Side", "Left", "--Label", args.Label])

				call_and_print([ComputeCSFVolume, "--VisitingMap", os.path.join("RH_Directory", args.Label + "_RH_Visitation.nrrd"), "--CSFProb",\
				os.path.join("RH_Directory", args.Label + "_CSF_Probability_Map.nrrd"),"--CSFFile", os.path.join("RH_Directory", args.Label\
				+ "_RH_" + surface + "_CSF_Density_Final_Smoothed.txt") , "--Side", "Right", "--Label", args.Label])
			else :
				call_and_print([ComputeCSFVolume, "--VisitingMap", os.path.join("LH_Directory", args.Label + "_LH_Visitation.nrrd"), "--CSFProb",\
				os.path.join("LH_Directory", args.Label + "_CSF_Probability_Map.nrrd"),"--CSFFile", os.path.join("LH_Directory", args.Label\
				+ "_LH_" + surface + "_CSF_Density_Final.txt") , "--Side", "Left", "--Label", args.Label])

				call_and_print([ComputeCSFVolume, "--VisitingMap", os.path.join("RH_Directory", args.Label + "_RH_Visitation.nrrd"), "--CSFProb",\
				os.path.join("RH_Directory", args.Label + "_CSF_Probability_Map.nrrd"),"--CSFFile", os.path.join("RH_Directory", args.Label\
				+ "_RH_" + surface + "_CSF_Density_Final.txt") , "--Side", "Right", "--Label", args.Label])

			if (os.path.isfile("../Outputs.csv")):

				header = ["LH CSF Density", "RH CSF Density", "LH Visitation Map", "RH Visitation Map"]
				LH_CSF_Density = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_CSF_Density.vtk")
				RH_CSF_Density = os.path.join(args.Output_Directory, "LocalEACSF", "RH_Directory", args.Label + "_RH_" + surface + "_CSF_Density.vtk")
				LH_Visitation_Map = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_Visitation.nrrd")
				RH_Visitation_Map = os.path.join(args.Output_Directory, "LocalEACSF", "RH_Directory", args.Label + "_RH_Visitation.nrrd")
				LH_CSF_Density_Inflated = os.path.join(args.Output_Directory, "LocalEACSF", "LH_Directory", args.Label + "_LH_" + surface + "_Inflated.vtk")
				RH_CSF_Density_Inflated = os.path.join(args.Output_Directory, "LocalEACSF", "RH_Directory", args.Label + "_RH_" + surface + "_Inflated.vtk")
				line = [LH_CSF_Density, RH_CSF_Density, LH_Visitation_Map, RH_Visitation_Map ]	

				if(os.path.isfile(LH_CSF_Density_Inflated) and os.path.isfile(RH_CSF_Density_Inflated)):
					header = ["LH CSF Density", "RH CSF Density", "LH Visitation Map", "RH Visitation Map", "LH CSF Density Inflated", "RH CSF Density Inflated"]
					line = [LH_CSF_Density, RH_CSF_Density, LH_Visitation_Map, RH_Visitation_Map, LH_CSF_Density_Inflated, RH_CSF_Density_Inflated ]

				f = csv.writer(open('../Outputs.csv','a'))
				needs_header = os.stat('../Outputs.csv').st_size == 0
				if needs_header:
					f.writerow(header)
				f.writerow(line)	

	print("Local_EACSF finished",flush=True)
	sys.exit(0)


if (__name__ == "__main__"):
	parser = argparse.ArgumentParser(description='Compute Local EACSF Density')	
	parser.add_argument('--ComputeCSFVolume', type=str, help='ComputeCSFVolume executable path', default='@ComputeCSFVolume_PATH@')
	parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
	parser.add_argument('--python3', type=str, help='Python3 Executable Path', default='@python3_PATH@')
	parser.add_argument('--Use_75P_Surface', type=bool, help='use the 75P Surface as the input surface', default=@Use_75P_Surface@)
	parser.add_argument('--Smooth', type=bool, help='Smooth the CSF Density with a heat kernel smoothing', default=@Smooth@)
	parser.add_argument("--Label",type=str, help='Label of the case , ID for example', default="@Label@")

	args = parser.parse_args()
	main(args)

