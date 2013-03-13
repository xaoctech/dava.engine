#ifndef __CONTROL_ROTATION_TEST_H__
#define __CONTROL_ROTATION_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class ControlRotationTest : public TestTemplate<ControlRotationTest>
{
    
public:
	ControlRotationTest();

	virtual void LoadResources();
	virtual void UnloadResources();

    void TestFunction(PerfFuncData * data);
    
private:

    UIStaticText *description;
    UIControl *rotatedControl;
    float32 angle;
    
};


#endif // __CONTROL_ROTATION_TEST_H__
