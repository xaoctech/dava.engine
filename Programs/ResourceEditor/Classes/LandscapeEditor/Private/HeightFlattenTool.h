#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightFlattenTool final : public DAVA::BaseHeightEditTool
{
public:
    enum eBrushMode
    {
        MODE_STATIC,
        MODE_DYNAMIC
    };

    HeightFlattenTool(DAVA::LandscapeEditorSystemV2* system);
    void Activate(const DAVA::PropertiesItem& settings) override;
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    eBrushMode GetMode() const;
    void SetMode(eBrushMode newMode);

    eBrushMode mode = MODE_STATIC;

    DAVA_VIRTUAL_REFLECTION(HeightFlattenTool, BaseHeightEditTool);
};
