#ifndef __FILELIST_TEST_H__
#define __FILELIST_TEST_H__

#include "DAVAEngine.h"
#include "TestTemplate.h"

using namespace DAVA;

class FileListTest: public TestTemplate<FileListTest>
{
public:
	FileListTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void ResTestFunction(PerfFuncData * data);
	void DocTestFunction(PerfFuncData * data);
    
protected:
    
    void RecursiveCopy(const FilePath & src, const FilePath &dst);
};

#endif //__FILELIST_TEST_H__
