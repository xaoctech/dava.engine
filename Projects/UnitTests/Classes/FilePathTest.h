//
//  InputTest.h
//  TemplateProjectMacOS
//
//  Created by adebt on 1/29/13.
//
//

#ifndef __FILEPATH_TEST_H__
#define __FILEPATH_TEST_H__

#include "DAVAEngine.h"
#include "TestTemplate.h"

using namespace DAVA;

class FilePathTest: public TestTemplate<FilePathTest>
{
public:
	FilePathTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void TestFunction(PerfFuncData * data);

	
};

#endif //__FILEPATH_TEST_H__
