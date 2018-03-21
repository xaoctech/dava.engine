#include "Classes/LandscapeEditor/Private/HeightSmoothTool.h"

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

HeightSmoothTool::HeightSmoothTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_smooth.png"), QString("Smooth height")),
                         "HeightSmoothTool")
{
    inputController->RegisterVarCallback(DAVA::eInputElements::KB_K, [this](const DAVA::Vector2& delta) {
        kernelSize = DAVA::Saturate(kernelSize + delta.y * 0.1);
    });
}

void HeightSmoothTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    kernelSize = toolSettings.Get<DAVA::float32>("kernelSize", kernelSize);
}

QWidget* HeightSmoothTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    {
        ControlDescriptorBuilder<Slider::Fields> fields;
        fields[Slider::Fields::Value] = "kernelSize";
        builder.RegisterParam<Slider>("Kernel size (K)", fields);
    }
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightSmoothTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Asset<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        DAVA::Vector4 params(strength, kernelSize, 0.0, 0.0);

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHT_SMOOTH"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightSmoothTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params(strength, kernelSize / static_cast<float32>(2 * floatTexture->width), 0.0f, 0.0f);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightSmoothTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("kernelSize", kernelSize);
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightSmoothTool)
{
    DAVA::ReflectionRegistrator<HeightSmoothTool>::Begin()[BaseLandscapeTool::SortKey(30)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("kernelSize", &HeightSmoothTool::kernelSize)[DAVA::M::Range(0.0f, 1.0f, 0.001f)]
    .End();
}
