#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightSmoothTool final : public DAVA::BaseHeightEditTool
{
public:
    HeightSmoothTool(DAVA::LandscapeEditorSystemV2* system);
    void Activate(const DAVA::PropertiesItem& settings) override;
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    DAVA::float32 kernelSize = 0.3f;

    DAVA_VIRTUAL_REFLECTION(HeightSmoothTool, BaseHeightEditTool);
};
