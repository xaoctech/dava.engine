#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightSetTool final : public DAVA::BaseHeightEditTool
{
public:
    HeightSetTool(DAVA::LandscapeEditorSystemV2* system);
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;

private:
    DAVA_VIRTUAL_REFLECTION(HeightSetTool, BaseHeightEditTool);
};
