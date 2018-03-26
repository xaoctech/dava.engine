#include "Tests/ImGuiTest.h"
#include "Infrastructure/TestBed.h"

#include <Engine/Engine.h>
#include <Engine/EngineSettings.h>
#include <Debug/Private/ImGui.h>
#include <UI/Update/UIUpdateComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include "UI/UIControlBackground.h"

ImGuiTest::ImGuiTest(TestBed& app)
    : BaseScreen(app, "ImGuiTest")
{
    backColor = DAVA::Color(.44f, .44f, .6f, 1.f);
}

void ImGuiTest::LoadResources()
{
    BaseScreen::LoadResources();

    GetOrCreateComponent<DAVA::UIUpdateComponent>();
    DAVA::UIControlBackground* bg = GetOrCreateComponent<DAVA::UIControlBackground>();

    bg->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    bg->SetColor(backColor);
}

void ImGuiTest::Update(DAVA::float32 timeElapsed)
{
    DVASSERT(ImGui::IsInitialized());

    if (ImGui::IsInitialized())
    {
        // 1. Show a simple window
        {
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(400, 180), ImGuiCond_FirstUseEver);

            ImGui::Begin("Simple Window", &showAnotherWindow);

            static float f = 0.0f;
            ImGui::Text("Hello, world!");
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
            if (ImGui::ColorEdit3("clear color", backColor.color))
                GetComponent<DAVA::UIControlBackground>()->SetColor(backColor);
            if (ImGui::Button("Test Window"))
                showTestWindow ^= 1;
            if (ImGui::Button("Another Window"))
                showAnotherWindow ^= 1;
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        // 2. Show another simple window, this time using an explicit Begin/End pair
        if (showAnotherWindow)
        {
            ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
            ImGui::Begin("Another Window", &showAnotherWindow);
            ImGui::Text("Hello");
            ImGui::End();
        }

        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
        if (showTestWindow)
        {
            ImGui::SetNextWindowPos(ImVec2(450, 20), ImGuiCond_FirstUseEver);
            ImGui::ShowDemoWindow();
        }
    }
}
