#include "Debug/Private/DebugOverlayItemEngineSettings.h"

#include <Engine/Engine.h>
#include <Engine/EngineSettings.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Debug/DebugOverlay.h>
#include <Debug/Private/ImGui.h>

namespace DAVA
{
DebugOverlayItemEngineSettings::DebugOverlayItemEngineSettings()
{
#if 0
    EngineSettings* engineSettings = GetEngineContext()->settings;
    for (size_t i = 0; i < 20; i++)
    {
        engineSettings->RegisterVar(FastName(Format("int8-%d", i).c_str()), i, Format("Help for int8-%d", i));
        engineSettings->RegisterVar(FastName(Format("int-%d", i).c_str()), i, Format("Help for int-%d", i), M::Range(5, 10, 1));
        engineSettings->RegisterVar(FastName(Format("float-%d", i).c_str()), static_cast<float32>(i), Format("Help for float-%d", i));
    }
#endif
}

String DebugOverlayItemEngineSettings::GetName() const
{
    return "EngineSettings";
}

void DebugOverlayItemEngineSettings::Draw(bool* shown, float32 timeElapsed)
{
    ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 250.0f), ImVec2(FLOAT_MAX, FLOAT_MAX));
    if (ImGui::Begin(GetName().c_str(), shown, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        EngineSettings* engineSettings = GetEngineContext()->settings;
        size_t engineSettingsSize = engineSettings->GetVarsCount();

        if (ImGui::InputText("Filter", filterBuf.data(), filterBuf.size(), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll, nullptr, (void*)this))
        {
        }

        ImGui::Separator();

        ImGui::BeginChild("Settings");
        ImGui::Columns(2);

        ImGuiListClipper clipper(static_cast<int>(engineSettingsSize));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                EngineSettingsVar* var = engineSettings->GetVar(i);

                ImGui::Text(var->GetName().c_str());
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip(var->GetHelp().c_str());

                ImGui::NextColumn();
                ImGui::PushID(var);

                const Any& value = var->GetValue();
                const Type* valueType = value.GetType();

                if (valueType->Is<bool>())
                {
                    bool v = value.Get<bool>();
                    if (EditBool(var, &v))
                    {
                        var->SetValue(v);
                    }
                }
                else if (valueType->IsEnum())
                {
                    int v = value.Cast<int>();
                    if (EditEnum(var, &v))
                    {
                        var->SetValueWithCast(v);
                    }
                }
                else if (valueType->IsIntegral() && valueType->GetSize() <= sizeof(int))
                {
                    int v = value.Cast<int>();
                    if (EditIntegral(var, &v))
                    {
                        var->SetValueWithCast(v);
                    }
                }
                else if (valueType->IsFloatingPoint())
                {
                    float32 v = value.Get<float32>();
                    if (EditFloat(var, &v))
                    {
                        var->SetValue(v);
                    }
                }
                else
                {
                    ImGui::Text("?");
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip(valueType->GetName());
                }

                ImGui::PopID();
                ImGui::NextColumn();
            }
        }

        ImGui::Columns(1);
        ImGui::EndChild();
    }
    ImGui::End();
}

bool DebugOverlayItemEngineSettings::EditBool(EngineSettingsVar* var, bool* v)
{
    return ImGui::Checkbox("", v);
}

bool DebugOverlayItemEngineSettings::EditEnum(EngineSettingsVar* var, int* v)
{
    return false;
}

bool DebugOverlayItemEngineSettings::EditIntegral(EngineSettingsVar* var, int* v)
{
    const M::Range* range = nullptr;

    const ReflectedMeta* meta = var->GetMeta();
    if (nullptr != meta)
    {
        range = meta->GetMeta<M::Range>();

        // TODO:
        // more metas here
        // ...
    }

    if (nullptr != range)
    {
        return ImGui::SliderInt("", v, range->minValue.Cast<int>(), range->maxValue.Cast<int>());
    }
    else
    {
        return ImGui::InputInt("", v);
    }
}

bool DebugOverlayItemEngineSettings::EditFloat(EngineSettingsVar* var, float32* v)
{
    return ImGui::InputFloat("", v);
}
}
