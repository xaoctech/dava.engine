#pragma once

#include "Infrastructure/BaseScreen.h"
#include <Base/RefPtr.h>
#include <UI/UIControl.h>
#include <UI/UITextField.h>

class TestBed;
class RichInputDelegate;
struct TextTestCase;

class TextSystemTest : public BaseScreen
{
    enum eState
    {
        STOPPED,
        PLAYING
    };

public:
    TextSystemTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;
    void Update(DAVA::float32 delta) override;

    void ChangeCurrentTest(int32 testIdx_);
    void Benchmark1();
    void Benchmark2();

private:
    eState state = STOPPED;
    UIStaticText* statusText;
    UIControl* holderControl;
    UIStaticText* benchmark1Btn;
    UIStaticText* benchmark2Btn;

    int32 testIdx;
    Vector<std::shared_ptr<TextTestCase>> objects;
    std::shared_ptr<TextTestCase> activeObject;
};
