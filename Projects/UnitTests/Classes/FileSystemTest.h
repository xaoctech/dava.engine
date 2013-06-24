#ifndef __FILESYSTEM_TEST_H__
#define __FILESYSTEM_TEST_H__

#include "DAVAEngine.h"
#include "TestTemplate.h"

using namespace DAVA;

class FileSystemTest: public TestTemplate<FileSystemTest>
{
public:
	FileSystemTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void ResTestFunction(PerfFuncData * data);
	void DocTestFunctionCheckCopy(PerfFuncData * data);
	void DocTestFunction(PerfFuncData * data);
    
protected:
    
    bool RecursiveCopy(const FilePath & src, const FilePath &dst);
};

#endif //__FILESYSTEM_TEST_H__
