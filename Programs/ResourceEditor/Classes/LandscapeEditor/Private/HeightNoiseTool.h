#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightNoiseTool final : public DAVA::BaseHeightEditTool
{
public:
    enum eMode
    {
        MODE_UNSIGNED = 0,
        MODE_SIGNED = 1,
    };

    HeightNoiseTool(DAVA::LandscapeEditorSystemV2* system);
    void Activate(const DAVA::PropertiesItem& settings) override;
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    DAVA::Vector4 GetParams() const;
    DAVA::Vector4 GetRandom() const;

    eMode GetMode() const;
    void SetMode(eMode newMode);

    eMode mode = MODE_UNSIGNED;
    DAVA::float32 kernelSize = 0.1f;
    DAVA::RefPtr<DAVA::Texture> noiseTexture;

    DAVA_VIRTUAL_REFLECTION(HeightNoiseTool, BaseHeightEditTool);
};
