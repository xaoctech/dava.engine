#!/usr/bin/python

import datetime

#temporary solution
import sys
sys.path.insert(1, '/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages')

#start work with mongo
import pymongo
import bson

arguments = sys.argv[1:]

if 0 == len(arguments) or 2 != len(arguments):
	print 'Usage: ./dbparser.py [CollectionName] [PlatformName]'
	exit(1)

collectionName = arguments[0]
platformName = arguments[1]
# platformName = 'MacOS'
# platformName = 'iPhone'
# platformName = 'Win32'
# platformName = 'Android'


def LogError(logfile, message):
	logfile.write('<br/><font color="red">')
	logfile.write(message)
	logfile.write('</font>')


report = open('!report.html', 'w')
report.write('<!DOCTYPE html>')
report.write('<html> <head>')
report.write('<title> PerformanceTest Report </title>')
report.write('</head> <body>')


# connection = pymongo.Connection("localhost", 27017)
connection = pymongo.Connection("10.128.128.131", 27017)

if None != connection:
	#set database
	db = connection['PerformanceTest']
	report.write('<br/> DataBase: ' + db.name)

	#set collection
	collection = db[collectionName]
	report.write('<br/> Collection: ' + collection.name)

	#get cursor for collection
	platform = collection.find_one({'_id': platformName})
	if None != platform:
		screenNames = platform.keys()
		for screenName in screenNames:
			if '_id' != screenName:
				report.write('<H1> Screen ' + screenName + ' </H1>')
				report.write('<table style="border: 1px solid gray; cellspacing: 0px; cellpadding: 0px; border-collapse: collapse;">')

				screen = platform[screenName]
				testNames = screen.keys()
				for testName in testNames:
					if '_id' != testName:
						report.write('<tr style="border: 1px solid gray; cellspacing: 0px; cellpadding: 0px;">')
						report.write('<td style="border: 1px solid gray; cellspacing: 0px; cellpadding: 0px;">')
						report.write(testName)
						report.write('</td>')

						test = screen[testName]
						testResultsNames = test.keys()
				
						prevAverage = -1;
						prevMin = -1
						prevMax = -1
						for testResultsName in testResultsNames:
							if "_id" != testResultsName:
								testData = test[testResultsName]

								averageValue = testData['Average']
								minValue = testData['MinTime']
								maxValue = testData['MaxTime']

								def PrintDelta(prevValue, newValue):
									if prevValue != -1:
								
										if prevValue != 0:
											delta = (float(newValue) - float(prevValue)) / float(prevValue)

											sign = ''
											color = 'black'
											if 0 < delta:
												sign = '+'
												color = 'red'
											elif delta < 0:
												color = 'green'

											report.write("<font size = '2' color = '" +color + "'>")
											report.write(sign+'%(percent).2f%%' % {'percent': delta*100})
											report.write('</font>')
									
										else:
											report.write('&nbsp')
									else:
										report.write('&nbsp')
								
								report.write('<td style="border: 1px solid gray; cellspacing: 0px; cellpadding: 0px;">')

								#internal table for test data
								report.write('<table style="width:100%;">')
								report.write('<tr>')

								report.write('<td style="cellpadding: 0px; width:33%">')
								report.write('%.2f'%float(minValue))
								report.write('</td>')

								report.write('<td style="cellpadding: 0px; width:33%">')
								report.write('%.2f'%float(maxValue))
								report.write('</td>')

								report.write('<td style="cellpadding: 0px; width:34%">')
								report.write('%.2f'%float(averageValue))
								report.write('</td>')
						

								report.write('</tr>')
								report.write('<tr>')

								report.write('<td>')
								PrintDelta(prevMin, minValue)
								report.write('</td>')

								report.write('<td>')
								PrintDelta(prevMax, maxValue)
								report.write('</td>')

								report.write('<td>')
								PrintDelta(prevAverage, averageValue)
								report.write('</td>')


								report.write('</tr>')
								report.write('</table>')

								report.write('</td>')
						
								prevMin = minValue
								prevMax = maxValue
								prevAverage = averageValue

						report.write('</tr>')

				report.write('</table>')
	else:
		LogError(report, "There are no tests for platform " + platformName + " at collection " + collectionName)
else:
	LogError(report, "Can't connect to Database")
	
report.write('</body>')
report.close()