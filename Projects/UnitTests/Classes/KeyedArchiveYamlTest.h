
#ifndef TemplateProjectiPhone_KeyedArchiveYamlTest_h
#define TemplateProjectiPhone_KeyedArchiveYamlTest_h

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class KeyedArchiveYamlTest : public TestTemplate<KeyedArchiveYamlTest>
{
public:
	KeyedArchiveYamlTest();
    
	virtual void LoadResources();
	virtual void UnloadResources();
    
    void PerformTest(PerfFuncData * data);
private:
    KeyedArchive loadedArchive;
};

#endif
