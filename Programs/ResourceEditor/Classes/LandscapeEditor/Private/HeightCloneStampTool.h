#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

class HeightCloneStampTool final : public DAVA::BaseHeightEditTool
{
public:
    HeightCloneStampTool(DAVA::LandscapeEditorSystemV2* system);

    void Activate(const DAVA::PropertiesItem& settings) override;
    DAVA::Asset<DAVA::Texture> GetCustomCoverTexture() const override;
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    void Process(DAVA::float32 delta) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    DAVA::Asset<DAVA::Texture> coverTexture;
    DAVA::RefPtr<DAVA::NMaterial> coverTextureGenerateMaterial;

    DAVA_VIRTUAL_REFLECTION(HeightCloneStampTool, BaseHeightEditTool);
};
