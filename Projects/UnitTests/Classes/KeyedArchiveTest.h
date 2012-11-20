
#ifndef TemplateProjectiPhone_KeyedArchiveTest_h
#define TemplateProjectiPhone_KeyedArchiveTest_h

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class KeyedArchiveTest : public TestTemplate<KeyedArchiveTest>
{
public:
	KeyedArchiveTest();
    
	virtual void LoadResources();
	virtual void UnloadResources();
    
    void WriteArchive(PerfFuncData * data);
    void LoadNewArchive(PerfFuncData * data);
    void LoadOldArchive(PerfFuncData * data);
    void TestArchiveAccordingDefines(PerfFuncData * data);
	    
private:
    
    void FillArchive(KeyedArchive *arch);

    KeyedArchive archiveToSave;
    KeyedArchive loadedArchive;
};

#endif
