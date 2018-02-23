#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

#include <Base/BaseTypes.h>

class HeightPushPullTool final : public DAVA::BaseHeightEditTool
{
public:
    HeightPushPullTool(DAVA::LandscapeEditorSystemV2* system);
    QWidget* CreateEditorWidget(const DAVA::BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;

private:
    class PushPullInputController;
    DAVA::float32 delta = 0.0;
    DAVA_VIRTUAL_REFLECTION(HeightPushPullTool, BaseHeightEditTool);
};
