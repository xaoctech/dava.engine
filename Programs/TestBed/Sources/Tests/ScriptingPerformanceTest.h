#pragma once

#include "Infrastructure/BaseScreen.h"
#include "Functional/Function.h"
#include "Base/Any.h"

class ScriptingPerformanceTest : public BaseScreen
{
public:
    ScriptingPerformanceTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 timeElapsed) override;

    void RunVSTest();
    void RunLuaTest();

private:
    DAVA::RefPtr<DAVA::UITextField> countArgText;
    DAVA::RefPtr<DAVA::UIStaticText> outputText;
    DAVA::RefPtr<DAVA::UIControl> container;
    DAVA::RefPtr<DAVA::UIStaticText> timeVsText;
    DAVA::RefPtr<DAVA::UIStaticText> timeLuaText;
};
