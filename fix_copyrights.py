#!/usr/bin/env python2.6

import os;
import sys;
import os.path;
import pickle;
import zlib;
import string;
import sys;
import subprocess;
import platform;
import re;

excludeDirs = ["Box2D", "Freetype", "Yaml", "ColladaConverter", "ThirdPartyLibs", "Libs", "yaml-cpp"]
includePaths = {}

replaceString = "/*===\n\n\n===*/\n";
	
excludeLogFile = open("excludeLog.log", "w");
includeLogFile = open("includeLog.log", "w");



def visit_directory(arg, dirname, names):
	global excludeDirs, includePaths
	# (path, name) = os.path.split(dirname);
	if (string.find(dirname, "$process") != -1):
		return;
	if (string.find(dirname, ".svn") != -1):
		return;
	if (string.find(dirname, ".git") != -1):
		return;
	relPath = os.path.relpath(dirname)
	for exDir in excludeDirs:	
		if (string.find(relPath, exDir) != -1):
			excludeLogFile.write("exclude: " + relPath + "\n");
			#print relPath;
			return;
	#print "include dir: " + relPath
	includeLogFile.write("include: " + relPath + "\n");

	(dirhead, dirtail) = os.path.split(dirname);
	fullpath = os.path.normpath( dirname + "/");
	for fullname in names:
		pathname = fullpath + "/" + fullname;
		if os.path.isdir(pathname): 
			continue;
		if fullname[0] == '.' or fullname[0] == '$':
			continue;
		includePaths[fullname] = os.path.relpath(pathname);		
		supported_exts = [".cpp", ".h", ".hpp"];
	return
	
def process_contents(content):
	pattern = re.compile("^/[*]=.*=[*]/", re.DOTALL);

	replacedContent = re.subn(pattern, replaceString, content, count=1);

	newContent = replacedContent[0];
	if(replacedContent[1] == 0):
		newContent = replaceString + "\n\n" + newContent;
		
	return newContent;
	
def process_file(fullname):
	f = open(fullname)
	contents = "";
	try:
		contents = f.read();
	finally:
	    f.close()
	
	contents = process_contents(contents);
	
	f = open(fullname, "wt")
	try: 
		f.write(contents);
	finally:
		f.close();
	
	return;
	
def process_files(arg, dirname, names):
	global excludeDirs, includePaths
	# (path, name) = os.path.split(dirname);
	if (string.find(dirname, "$process") != -1):
		return;
	if (string.find(dirname, ".svn") != -1):
		return;
	relPath = os.path.relpath(dirname)
	for exDir in excludeDirs:	
		if (string.find(relPath, exDir) != -1):
			excludeLogFile.write("exclude: " + relPath + "\n");
			#print relPath
			return;
	#print "include dir: " + relPath
	includeLogFile.write("include: " + relPath + "\n");

	(dirhead, dirtail) = os.path.split(dirname);
	fullpath = os.path.normpath( dirname + "/");
	for fullname in names:
		pathname = fullpath + "/" + fullname;
		if os.path.isdir(pathname): 
			continue;
		if fullname[0] == '.' or fullname[0] == '$':
			continue;
		
		(name, ext) = os.path.splitext(fullname); 
		supported_exts = [".cpp", ".h", ".hpp"];
		if ext in supported_exts:
			process_file(pathname);
			
	return
	
export_script_dir = os.getcwd();
os.path.walk(export_script_dir, visit_directory, None);
os.path.walk(export_script_dir, process_files, None);

excludeLogFile.close();
includeLogFile.close();

#process_file("Animation/AnimatedObject.cpp")