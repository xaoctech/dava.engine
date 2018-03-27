#include "Tests/ScriptingPerformanceTest.h"
#include "Base/Type.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scripting/LuaScript.h"
#include "UI/Events/UIEventBindingComponent.h"
#include "UI/VisualScript/UIVisualScriptComponent.h"
#include "UI/VisualScript/UIVisualScriptSystem.h"
#include "UI/Script/UIScriptComponent.h"
#include "UI/Script/UIScriptSystem.h"
#include "Utils/StringUtils.h"

using namespace DAVA;

ScriptingPerformanceTest::ScriptingPerformanceTest(TestBed& app)
    : BaseScreen(app, "ScriptingPerformanceTest")
{
}

void ScriptingPerformanceTest::LoadResources()
{
    BaseScreen::LoadResources();

    DAVA::DefaultUIPackageBuilder pkgBuilder;
    DAVA::UIPackageLoader().LoadPackage("~res:/TestBed/UI/ScriptingPerformanceTest.yaml", &pkgBuilder);
    UIControl* dialog = pkgBuilder.GetPackage()->GetControl("MainFrame");
    AddControl(dialog);

    countArgText = static_cast<UITextField*>(dialog->FindByName("CountArgText"));
    outputText = static_cast<UIStaticText*>(dialog->FindByName("OutputText"));
    container = static_cast<UIStaticText*>(dialog->FindByName("TestContainer"));
    timeVsText = static_cast<UIStaticText*>(dialog->FindByName("TimeVs"));
    timeLuaText = static_cast<UIStaticText*>(dialog->FindByName("TimeLua"));

    countArgText->SetUtf8Text("100");
    outputText->SetUtf8Text("");

    auto actions = dialog->GetOrCreateComponent<UIEventBindingComponent>();
    actions->BindAction(FastName("RUN_VS_SCRIPT"), [&]() { RunVSTest(); });
    actions->BindAction(FastName("RUN_LUA_SCRIPT"), [&]() { RunLuaTest(); });
}

void ScriptingPerformanceTest::UnloadResources()
{
    countArgText.Set(nullptr);
    outputText.Set(nullptr);

    BaseScreen::UnloadResources();
}

void ScriptingPerformanceTest::Update(DAVA::float32 timeElapsed)
{
}

void ScriptingPerformanceTest::RunVSTest()
{
    container->RemoveAllControls();
    RefPtr<UIControl> control(new UIControl(Rect(0, 0, 50, 50)));
    container->AddControl(control);
    UIVisualScriptComponent* vs = control->GetOrCreateComponent<UIVisualScriptComponent>();
    // vs->SetScriptPath("~res:/TestBed/VScripts/Performance_01.dvs");//Basic
    vs->SetScriptPath("~res:/TestBed/VScripts/Performance_02.dvs"); //LoopTest

    UIVisualScriptSystem* vsSys = Engine::Instance()->GetContext()->uiControlSystem->GetSystem<UIVisualScriptSystem>();

    int32 count = atoi(countArgText->GetUtf8Text().c_str());
    vsSys->Process(100);

    try
    {
        uint64 begin = SystemTimer::GetUs();

        for (int32 i = 0; i < count; i++)
        {
            vsSys->Process(100);
        }

        uint64 time = SystemTimer::GetUs() - begin;

        String output = "Done.";
        outputText->SetUtf8Text(output);
        timeVsText->SetUtf8Text(Format("Time Avg: %llu us, Total: %llu us", time / count, time));
    }
    catch (...)
    {
        String error = "Script failed!";
        Logger::Error(error.c_str());
        outputText->SetUtf8Text(error);
        timeVsText->SetUtf8Text("Error");
    }
    container->RemoveAllControls();
}

void ScriptingPerformanceTest::RunLuaTest()
{
    container->RemoveAllControls();
    RefPtr<UIControl> control(new UIControl(Rect(0, 0, 50, 50)));
    container->AddControl(control);
    UIScriptComponent* vs = control->GetOrCreateComponent<UIScriptComponent>();
    //vs->SetLuaScriptPath("~res:/TestBed/Lua/Components/Performance_01.lua");//Basic
    vs->SetLuaScriptPath("~res:/TestBed/Lua/Components/Performance_02.lua"); //LoopTest

    UIScriptSystem* vsSys = Engine::Instance()->GetContext()->uiControlSystem->GetSystem<UIScriptSystem>();

    int32 count = atoi(countArgText->GetUtf8Text().c_str());
    vsSys->Process(100);

    try
    {
        uint64 begin = SystemTimer::GetUs();

        for (int32 i = 0; i < count; i++)
        {
            vsSys->Process(100);
        }

        uint64 time = SystemTimer::GetUs() - begin;

        String output = "Done.";
        outputText->SetUtf8Text(output);
        timeLuaText->SetUtf8Text(Format("Time Avg: %llu us, Total: %llu us", time / count, time));
    }
    catch (...)
    {
        String error = "Script failed!";
        Logger::Error(error.c_str());
        outputText->SetUtf8Text(error);
        timeLuaText->SetUtf8Text("Error");
    }
    container->RemoveAllControls();
}
