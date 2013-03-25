#!/usr/bin/python

import pymongo
import bson
import yaml
import sys
arguments = sys.argv[1:]

if 0 == len(arguments) or 1 != len(arguments):
	print 'Usage: ./LevelPerformanceTestReport.py [Test ID]'
	exit(1)

testID = arguments[0]


def LogError(logfile, message):
	logfile.write('<br/><font color="red">')
	logfile.write(message)
	logfile.write('</font>\n')


stream = open('Data\Config\config.yaml', 'r')
config = yaml.load(stream)

report = open('report.html', 'w')
report.write('<!DOCTYPE html>\n')
report.write('<html>\n <head>\n')
report.write('<title> Level Performance Test Report, Test ID: ' + testID + '</title>\n')
report.write('</head>\n <body style="font-family: calibri;">\n')
report.write('<H1> Level Performance Test Report</H1>\nTest ID: ' + testID + '\n')

connection = None;
try:
	connection = pymongo.Connection("by2-buildmachine.wargaming.net", 27017)
except:
	connection = None

if None != connection:

	db = connection['LevelPerformanceTests']
	collection = db['LevelPerformanceTestsResult']
	
	currTest = collection.find_one({'_id': testID})
	if None != currTest:
		report.write('<H3> Device: ' + currTest['DeviceDescription'] + '</H3></br>\n')
		
		
		report.write('<table border="3" cellspacing="2"><tr>\n')
		for var in config['list']:
			color = '#'
			for c in var[1]:
				color += '{0:02x}'.format(c)
			report.write('<td bgcolor="' + color + '" height="50" width="50"/>\n')
			
		report.write('</tr><tr>\n')
		
		colorsCount = len(config['list'])
		for i in range(0, colorsCount):
			if i < (colorsCount - 1):
				report.write('<td align="center">' + str(config['list'][i][0]) + ' - ' + str(config['list'][i+1][0]) + '</br>fps</td>\n')
			else:
				report.write('<td align="center">'  + str(config['list'][i][0]) + '+ </br>fps</td>\n')
			
		report.write('</tr></table></br>\n')
		
		levelNames = currTest.keys()
		levelNames.sort()
		for levelName in levelNames:
			if '_id' != levelName and 'globalIndex' != levelName and 'DeviceDescription' != levelName:
				report.write('<H2> Level: ' + levelName + ' </H2>\n')

				level = currTest[levelName]
				reportValues = level.keys()
				reportValues.sort()
				for reportValue in reportValues:
					if '_id' != reportValue and 'ResultImagePNG' != reportValue:
						report.write('<b><i>' + reportValue + '</i></b>: ' + level[reportValue] + '<br/>\n')
						
				imageFile = open(levelName + '.png', 'wb')
				imageFile.write(level['ResultImagePNG'])
				report.write('<img src="./' + levelName + '.png"' + ' alt="'+ levelName +'"></br>\n')
	else:
		LogError(report, "There are no test with ID: " + testID)
		
#	collection.remove({"_id": testID})
	
else:
	LogError(report, "Can't connect to Database")
	
report.write('</body>')
report.close()
