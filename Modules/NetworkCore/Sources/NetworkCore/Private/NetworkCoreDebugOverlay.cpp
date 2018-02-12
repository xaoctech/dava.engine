#include "NetworkCore/Private/NetworkCoreDebugOverlay.h"

#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Debug/DebugOverlay.h"
#include "Debug/Private/ImGui.h"

namespace DAVA
{
String NetworkCoreDebugOverlayItem::GetName() const
{
    return "NetworkCore";
}

void NetworkCoreDebugOverlayItem::Draw()
{
    bool isShown = true;

    ImGui::SetNextWindowSizeConstraints(ImVec2(100.0f, 50.0f), ImVec2(FLOAT_MAX, FLOAT_MAX));
    if (ImGui::Begin("NetworkCore", &isShown, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        if (ImGui::CollapsingHeader("Snapshot misprediction"))
        {
            ImGui::Text("TODO NetworkCore");
        }

        // TODO:
        // ...
    }
    ImGui::End();

    if (!isShown)
    {
        GetEngineContext()->debugOverlay->HideItem(this);
    }

    ImGui::ShowTestWindow();
}
}
