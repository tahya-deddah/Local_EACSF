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


def main(args):
	#Executables
	python = args.python3
	#Output Path
	OUT_PATH = os.path.join(args.Output_Directory,"LocalEACSF") 	
	scripts_prefix = os.path.join(OUT_PATH, "PythonScripts")
	process_left_hemisphere = os.path.join(scripts_prefix,"process_left_hemisphere.py")
	process_right_hemisphere = os.path.join(scripts_prefix,"process_right_hemisphere.py")
	####
	Process_Left_Side = subprocess.call([python, process_left_hemisphere])
	#Process_Right_Side = subprocess.call([python, process_right_hemisphere])
	print("Local_EACSF finished",flush=True)
	sys.exit(0)


if (__name__ == "__main__"):
	parser = argparse.ArgumentParser(description='Compute Local EACSF Density')
	parser.add_argument("--Output_Directory",type=str, help='Output Directory', default="@Output_Directory@")
	parser.add_argument('--python3', type=str, help='Python3 Executable Path', default='@python3_PATH@')
	args = parser.parse_args()
	main(args)



