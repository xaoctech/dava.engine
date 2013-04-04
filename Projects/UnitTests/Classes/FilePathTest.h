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

	void MacTestFunction(PerfFuncData * data);
	void WinTestFunction(PerfFuncData * data);

	
};

#endif //__FILEPATH_TEST_H__
