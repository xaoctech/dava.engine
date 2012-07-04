#!/usr/bin/python

import pymongo
import bson

import sys
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


report = open('report.html', 'w')
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

	report.write('<br/> Delta formula is: delta = (newValue - prevValue) / prevValue')

	#get cursor for collection
	platform = collection.find_one({'_id': platformName})
	if None != platform:
		screenNames = platform.keys()
		screenNames.sort()
		for screenName in screenNames:
			if '_id' != screenName and 'globalIndex' != screenName:
				report.write('<H1> Screen: ' + screenName + ' </H1>')

				screen = platform[screenName]
				testNames = screen.keys()
				testNames.sort()
				for testName in testNames:
					if '_id' != testName:
						
						report.write('<H3> Test: ' + testName + ' </H3>')
						
						report.write('<table style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px; border-collapse: collapse;">')

						report.write('<tr style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						
						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Run Time')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Min Time')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Max Time')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Average Time')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Min delta')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Max delta')
						report.write('</td>')

						report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
						report.write('Average delta')
						report.write('</td>')

						report.write('</tr>')

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
								
								report.write('<tr style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								
								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								report.write(testData['RunTime'])
								report.write('</td>')
								
								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								report.write('%.2f'%float(minValue))
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								report.write('%.2f'%float(maxValue))
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								report.write('%.2f'%float(averageValue))
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevMin, minValue)
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevMax, maxValue)
								report.write('</td>')

								report.write('<td style="border: 1px solid gray; cellspacing: 1px; cellpadding: 1px;">')
								PrintDelta(prevAverage, averageValue)
								report.write('</td>')
								
								report.write('</tr>')
								
								prevMin = minValue
								prevMax = maxValue
								prevAverage = averageValue

						report.write('</table>')
	else:
		LogError(report, "There are no tests for platform " + platformName + " at collection " + collectionName)
else:
	LogError(report, "Can't connect to Database")
	
report.write('</body>')
report.close()