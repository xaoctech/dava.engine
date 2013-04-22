#ifndef __TEMPLATEPROJECTMACOS__LOCALIZATIONTEST__
#define __TEMPLATEPROJECTMACOS__LOCALIZATIONTEST__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class LocalizationTest : public TestTemplate<LocalizationTest>
{
	enum eConst
	{
		FIRST_TEST = 0,
		TEST_COUNT = 6
	};

public:
	LocalizationTest();

	virtual void LoadResources();
	virtual void UnloadResources();

    virtual void Draw(const UIGeometricData &geometricData);

    void TestFunction(PerfFuncData * data);

private:
    int32 currentTest;

	FilePath srcDir;
	FilePath cpyDir;

	bool CompareFiles(const FilePath& file1, const FilePath& file2);
};

#endif /* defined(__TEMPLATEPROJECTMACOS__LOCALIZATIONTEST__) */
