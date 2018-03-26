#pragma once

#include <Base/Array.h>
#include <Debug/DebugOverlayItem.h>

namespace DAVA
{
class EngineSettingsVar;
class DebugOverlayItemEngineSettings final : public DebugOverlayItem
{
public:
    DebugOverlayItemEngineSettings();

    String GetName() const override;
    void Draw(bool* shown, float32 timeElapsed) override;

private:
    Array<char, 128> filterBuf = {};

    bool EditBool(EngineSettingsVar* var, bool* v);
    bool EditEnum(EngineSettingsVar* var, int* v);
    bool EditIntegral(EngineSettingsVar* var, int* v);
    bool EditFloat(EngineSettingsVar* var, float32* v);
};
}
