#pragma once

#include "Base/BaseTypes.h"

#include <imgui/imgui.h>

namespace DAVA
{
struct InputEvent;
}

namespace ImGui
{
void Initialize();
bool IsInitialized();
void OnFrameBegin();
void OnFrameEnd();
bool OnInput(const DAVA::InputEvent& input);
void Uninitialize();

struct Settings
{
    static DAVA::float32 scale;
    static DAVA::float32 pendingScale;
    static DAVA::uint32 screenWidth;
    static DAVA::uint32 screenHeight;
};
} // namespace ImGui
