import os
import os.path
import shutil
import platform
import sys
import shutil

doxyfiles = ['Doxyfile','Doxyfile_chm']
cwd = os.getcwd()
for file in doxyfiles:
	if os.path.isfile(file):
		print "Parsing file: ",file
		#if os.path.isdir('doxydocs') != 1:
		if shutil.rmtree('doxydocs', 1) == 1 or os.path.isdir('doxydocs') != 1:
			os.system(('doxygen '+file))
			os.chdir(cwd)
os.chdir(cwd)
