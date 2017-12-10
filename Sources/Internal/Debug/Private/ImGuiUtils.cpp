#include "Debug/Private/ImGuiUtils.h"

#include "Debug/Private/ImGui.h"
#include "Engine/Engine.h"
#include "Render/2D/Systems/VirtualCoordinatesSystem.h"
#include "UI/UIControlSystem.h"

#include <imgui/imgui_internal.h>

namespace DAVA
{
void ImGuiUtils::SetScale(float32 scale)
{
    ImGui::Settings::scale = ImGui::Settings::scaleFromNextFrame = scale;
}

float32 ImGuiUtils::GetScale()
{
    return ImGui::Settings::scale;
}

void ImGuiUtils::SetScaleFromNextFrame(float32 scale)
{
    ImGui::Settings::scaleFromNextFrame = scale;
}

void ImGuiUtils::SetImGuiVirtualScreenSize(const Vector2& virtualScreenSize)
{
    ImGui::Settings::virtualScreenSize = virtualScreenSize;
}

Vector2 ImGuiUtils::GetImGuiVirtualScreenSize()
{
    return ImGui::Settings::virtualScreenSize;
}

float32 ImGuiUtils::GetImGuiVirtualToPhysicalScreenSizeScale()
{
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    Size2i screen = vcs->GetPhysicalScreenSize();
    float32 scaleX = static_cast<float32>(screen.dx) / ImGui::Settings::virtualScreenSize.dx;
    float32 scaleY = static_cast<float32>(screen.dy) / ImGui::Settings::virtualScreenSize.dy;

    return Min(scaleX, scaleY);
}

Vector2 ImGuiUtils::ConvertInputCoordsToImGuiVirtualCoords(const Vector2& inputCoords)
{
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    Vector2 coords = vcs->ConvertInputToVirtual(inputCoords);
    Vector2 physCoords = vcs->ConvertVirtualToPhysical(coords);

    return physCoords / GetImGuiVirtualToPhysicalScreenSizeScale();
}

Vector2 ImGuiUtils::ConvertPhysicalCoordsToImGuiVirtualCoords(const Vector2& physicalCoords)
{
    return physicalCoords / GetImGuiVirtualToPhysicalScreenSizeScale();
}

Vector2 ImGuiUtils::ConvertVirtualCoordsToImGuiVirtualCoords(const Vector2& virtualCoords)
{
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    Vector2 coords = vcs->ConvertVirtualToPhysical(virtualCoords);
    return ConvertPhysicalCoordsToImGuiVirtualCoords(coords);
}

Vector2 ImGuiUtils::ConvertImGuiVirtualCoordsToPhysicalCoords(const Vector2& virtualCoords)
{
    return virtualCoords * GetImGuiVirtualToPhysicalScreenSizeScale();
}

Vector2 ImGuiUtils::ConvertImGuiVirtualCoordsToVirtualCoords(const Vector2& virtualCoords)
{
    VirtualCoordinatesSystem* vcs = GetEngineContext()->uiControlSystem->vcs;

    Vector2 coords = ConvertImGuiVirtualCoordsToPhysicalCoords(virtualCoords);
    return vcs->ConvertPhysicalToVirtual(coords);
}
} // namespace DAVA