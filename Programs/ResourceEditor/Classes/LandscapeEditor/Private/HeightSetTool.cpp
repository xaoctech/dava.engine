#include "Classes/LandscapeEditor/Private/HeightSetTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/Slider.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Render/Highlevel/RenderPassNames.h>

HeightSetTool::HeightSetTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_set.png"), QStringLiteral("Set height")),
                         "HeightSetTool")
{
}

QWidget* HeightSetTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightSetTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::RefPtr<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        DAVA::Vector4 params;
        params.x = strength;

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHTMAP_SET"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture.Get());
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightSetTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params;
        params.x = strength;
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightSetTool)
{
    DAVA::ReflectionRegistrator<HeightSetTool>::Begin()[BaseLandscapeTool::SortKey(60)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .End();
}
