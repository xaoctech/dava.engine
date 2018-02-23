#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightSharpenTool final : public DAVA::BaseHeightEditTool
{
public:
    enum eMode
    {
        MODE_HIGH = 0,
        MODE_MEDIUM = 1,
        MODE_LOW = 2
    };

    HeightSharpenTool(DAVA::LandscapeEditorSystemV2* system);
    void Activate(const DAVA::PropertiesItem& settings) override;
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    eMode mode = MODE_HIGH;

    DAVA_VIRTUAL_REFLECTION(HeightSharpenTool, BaseHeightEditTool);
};
