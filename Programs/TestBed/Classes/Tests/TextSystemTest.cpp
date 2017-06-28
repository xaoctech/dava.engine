#include "Tests/TextSystemTest.h"

#include "Infrastructure/TestBed.h"
#include <Logger/Logger.h>
#include <Utils/Utils.h>
#include <Utils/UTF8Utils.h>
#include <UI/Update/UIUpdateComponent.h>
#include <UI/Text/UITextComponent.h>
#include <UI/Text/UITextSystem.h>
#include <UI/Input/UIActionBindingComponent.h>

using namespace DAVA;

const static std::vector<String> testCaseNames = {
    "NoMultilineTest",
    "MultilineTest",
    "MultilineBySymbolTest",
    "ShadowTest",
    "ParentColorTest",
    "PercentOfContentTest"
};

struct TextTestCase
{
    RefPtr<UIControl> control;
    String name;

    Vector2 origSize;

    float minScale = 0.8f;
    float maxScale = 1.2f;
    float scale = 1.f;
    float scaleStep = 0.05f;

    TextTestCase(UIControl* control_, String name_)
    {
        control = control_;
        name = name_;
        origSize = control->GetSize();
    }

    void Update(float32 delta)
    {
        scale += scaleStep * delta;
        if (scale > maxScale)
        {
            scale = maxScale;
            scaleStep = -scaleStep;
        }
        else if (scale < minScale)
        {
            scale = minScale;
            scaleStep = -scaleStep;
        }
        control->SetSize(origSize * scale);
    }
};

TextSystemTest::TextSystemTest(TestBed& app)
    : BaseScreen(app, "TextSystemTest")
{
    GetOrCreateComponent<UIUpdateComponent>();
}

void TextSystemTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/UI/Text/TextSystemTest.yaml", &pkgBuilder);
    UIControl* dialog = pkgBuilder.GetPackage()->GetControl("MainFrame");
    AddControl(dialog);

    statusText = static_cast<UIStaticText*>(dialog->FindByName("StatusText"));
    holderControl = static_cast<UIStaticText*>(dialog->FindByName("Holder"));
    benchmark1Btn = static_cast<UIStaticText*>(dialog->FindByName("Benchmark1Btn"));
    benchmark2Btn = static_cast<UIStaticText*>(dialog->FindByName("Benchmark2Btn"));

    UIActionMap& amap = dialog->GetOrCreateComponent<UIActionBindingComponent>()->GetActionMap();
    amap.Put(FastName("START"), [&]() {
        state = PLAYING;
        ChangeCurrentTest(testIdx);
    });
    amap.Put(FastName("STOP"), [&]() {
        state = STOPPED;
    });
    amap.Put(FastName("NEXT"), [&]() {
        ChangeCurrentTest(testIdx + 1);
    });
    amap.Put(FastName("PREV"), [&]() {
        ChangeCurrentTest(testIdx - 1);
    });
    amap.Put(FastName("BENCHMARK1"), [&]() {
        Benchmark1();
    });
    amap.Put(FastName("BENCHMARK2"), [&]() {
        Benchmark2();
    });

    for (String name : testCaseNames)
    {
        UIControl* c = pkgBuilder.GetPackage()->GetControl(name);
        objects.push_back(std::move(std::make_shared<TextTestCase>(c, name)));
    }
    activeObject = nullptr;
}

void TextSystemTest::UnloadResources()
{
    BaseScreen::UnloadResources();

    statusText = nullptr;
    activeObject = nullptr;
    holderControl = nullptr;
    benchmark1Btn = nullptr;
    benchmark2Btn = nullptr;
}

void TextSystemTest::Benchmark1()
{
    Vector<String> testTexts;
    String s = "qwe ";
    for (int32 i = 0; i < 10; i++)
    {
        s = s + s;
        testTexts.push_back(s);
    }

    uint64 begin = SystemTimer::GetUs();
    const int32 n = 100;
    for (int j = 0; j < n; j++)
    {
        UIStaticText* staticText = new UIStaticText(Rect(0.f, 0.f, 100.f, 100.f));
        AddControl(staticText);
        for (String ts : testTexts)
        {
            staticText->SetUtf8Text(s);
        }
        for (auto mType : { UIStaticText::MULTILINE_DISABLED, UIStaticText::MULTILINE_ENABLED, UIStaticText::MULTILINE_ENABLED_BY_SYMBOL })
        {
            staticText->SetMultilineType(mType);
        }
        Vector<int32> fittingMasks{ 0, TextBlock::FITTING_ENLARGE, TextBlock::FITTING_REDUCE, TextBlock::FITTING_ENLARGE | TextBlock::FITTING_REDUCE, TextBlock::FITTING_POINTS };
        for (int32 fitting : fittingMasks)
        {
            staticText->SetFittingOption(fitting);
        }
        RemoveControl(staticText);
        SafeRelease(staticText);
    }

    uint64 time = SystemTimer::GetUs() - begin;
    benchmark1Btn->SetUtf8Text(Format("UIStaticText %d updates: %u ms", n, time / 1000));
}

void TextSystemTest::Benchmark2()
{
    Vector<String> testTexts;
    String s = "qwe ";
    for (int32 i = 0; i < 10; i++)
    {
        s = s + s;
        testTexts.push_back(s);
    }
    UITextSystem* textSystem = UIControlSystem::Instance()->GetSystem<UITextSystem>();

    uint64 begin = SystemTimer::GetUs();
    const int32 n = 100;
    for (int j = 0; j < n; j++)
    {
        UIControl* control = new UIControl(Rect(0.f, 0.f, 100.f, 100.f));
        AddControl(control);
        UITextComponent* textComponent = control->GetOrCreateComponent<UITextComponent>();
        for (String ts : testTexts)
        {
            textComponent->SetText(s);
        }
        for (auto multiline : { UITextComponent::MULTILINE_DISABLED, UITextComponent::MULTILINE_ENABLED, UITextComponent::MULTILINE_ENABLED_BY_SYMBOL })
        {
            textComponent->SetMultiline(multiline);
        }
        Vector<UITextComponent::eTextFitting> fittingMasks{ UITextComponent::FITTING_NONE, UITextComponent::FITTING_ENLARGE, UITextComponent::FITTING_REDUCE, UITextComponent::FITTING_FILL, UITextComponent::FITTING_POINTS };
        for (UITextComponent::eTextFitting fitting : fittingMasks)
        {
            textComponent->SetFitting(fitting);
        }
        textSystem->ForceProcessControl(0.f, control);

        RemoveControl(control);
        SafeRelease(control);
    }

    uint64 time = SystemTimer::GetUs() - begin;
    benchmark2Btn->SetUtf8Text(Format("UITextComponent %d updates: %u ms", n, time / 1000));
}

void TextSystemTest::ChangeCurrentTest(int32 testIdx_)
{
    testIdx = DAVA::Clamp<int32>(testIdx_, 0, static_cast<int32>(objects.size() - 1));

    DVASSERT(objects.size() > 0);

    holderControl->RemoveAllControls();

    activeObject = objects[testIdx];
    if (activeObject)
    {
        activeObject->control->SetPivotPoint(Vector2(0.5f, 0.5f));
        holderControl->AddControl(activeObject->control.Get());
    }
    else
    {
        activeObject = nullptr;
    }
}

void TextSystemTest::Update(float32 delta)
{
    static float32 updateDelta = 0.f;
    static uint32 framesCount = 0;

    BaseScreen::Update(delta);

    updateDelta += SystemTimer::GetRealFrameDelta();
    framesCount += 1;
    if (updateDelta > 0.5f)
    {
        float32 fps = framesCount / updateDelta;
        const char* c_state = (state == PLAYING) ? "PLAYING" : "STOPPED";
        String testName = "<NULL>";
        if (activeObject)
        {
            testName = activeObject->name;
        }
        statusText->SetUtf8Text(Format("FPS: %f\nSTATE: %s\nTEST: %s", fps, c_state, testName.c_str()));
        updateDelta = 0.f;
        framesCount = 0;
    }

    if (activeObject && state == PLAYING)
    {
        activeObject->Update(delta);
    }
}
