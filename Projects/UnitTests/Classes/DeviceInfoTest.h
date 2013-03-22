#ifndef __TEMPLATEPROJECTMACOS__DEVICEINFOTEST__
#define __TEMPLATEPROJECTMACOS__DEVICEINFOTEST__

#include "DAVAEngine.h"

using namespace DAVA;

#include "TestTemplate.h"

class DeviceInfoTest : public TestTemplate<DeviceInfoTest>
{
public:
	DeviceInfoTest();
	
	virtual void LoadResources();
	virtual void UnloadResources();
	
    virtual void Draw(const UIGeometricData &geometricData);

    void TestFunction(PerfFuncData * data);
};

#endif /* defined(__TEMPLATEPROJECTMACOS__DEVICEINFOTEST__) */
