#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseHeightEditTool.h>

#include <Math/Vector.h>
#include <Base/RefPtr.h>

namespace DAVA
{
class RenderObject;
} // namespace DAVA

class HeightAverageTool final : public DAVA::BaseHeightEditTool
{
public:
    enum eBrushMode
    {
        MODE_CONTINUES,
        MODE_SINGLE
    };

    HeightAverageTool(DAVA::LandscapeEditorSystemV2* system);
    QWidget* CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params) override;
    void Activate(const DAVA::PropertiesItem& settings) override;
    void Process(DAVA::float32 delta) override;
    DAVA::Vector<BrushPhaseDescriptor> CreateBrushPhases() override;
    void PrepareBrushPhase(BrushPhaseDescriptor& phase) const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    DAVA::Vector2 GetUVPos() const;
    DAVA::Vector2 GetKernel() const;
    DAVA::Vector4 GetLandscapeParams() const;
    DAVA::Vector4 GetParams() const;

    DAVA::RefPtr<DAVA::RenderObject> normalDebugDrawGeometry;
    DAVA::RefPtr<DAVA::NMaterial> normalDebugDrawMaterial;

    eBrushMode mode = MODE_SINGLE;
    DAVA::float32 kernelSize = 0.1f;
    DAVA::float32 kernelStrength = 1.0f;

    DAVA_VIRTUAL_REFLECTION(HeightAverageTool, BaseHeightEditTool);
};
