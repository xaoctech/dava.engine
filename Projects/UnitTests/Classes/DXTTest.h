#ifndef __DXT_TEST_H__
#define __DXT_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

class DXTTest : public TestTemplate<DXTTest>
{
    enum eConst
    {
        FIRST_TEST = 0,
        TESTS_COUNT = 7,
        ACCETABLE_DELTA_IN_PERSENTS = 2
    };
    
public:
	DXTTest();

	virtual void LoadResources();
	virtual void UnloadResources();

    virtual void Draw(const UIGeometricData &geometricData);

    
    void TestFunction(PerfFuncData * data);
    
private:
    
    int32 currentTest;
    
    void ReloadSprites();
    bool IsCurrentTestAccepted();
    
    UIStaticText *compareResultText;
    
    Sprite *pngSprite;
    Sprite *dxtSprite;
    Sprite *decompressedPNGSprite;
};


#endif // __DXT_TEST_H__
