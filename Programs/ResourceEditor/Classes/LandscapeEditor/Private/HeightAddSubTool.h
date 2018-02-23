#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightAddSubTool final : public DAVA::BaseHeightEditTool
{
public:
    enum eBrushMode
    {
        MODE_ADD,
        MODE_SUB
    };

    HeightAddSubTool(DAVA::LandscapeEditorSystemV2* system);
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    void Activate(const DAVA::PropertiesItem& settings) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    eBrushMode GetMode() const;
    void SetMode(eBrushMode newMode);

    eBrushMode mode = MODE_ADD;

    DAVA_VIRTUAL_REFLECTION(HeightAddSubTool, BaseHeightEditTool);
};
