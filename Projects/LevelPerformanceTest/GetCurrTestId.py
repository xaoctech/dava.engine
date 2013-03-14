#!/usr/bin/python

import pymongo
import bson

import sys

report = open('testId', 'w')
	
connection = None;
try:
	connection = pymongo.Connection("by2-buildmachine.wargaming.net", 27017)
except:
	connection = None

if None != connection:

	db = connection['LevelPerformanceTests']
	collection = db['LevelPerformanceTestsResult']
	
	currTest = collection.find_one({'_id': 'GlobalTestId'})
	if None != currTest:
		report.write(str(currTest['LastTestId']))

report.close()
