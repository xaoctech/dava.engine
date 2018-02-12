#include "Classes/LandscapeEditor/Private/HeightSharpenTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Slider.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Base/GlobalEnum.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Render/Highlevel/RenderPassNames.h>

ENUM_DECLARE(HeightSharpenTool::eMode)
{
    ENUM_ADD_DESCR(HeightSharpenTool::MODE_HIGH, "High");
    ENUM_ADD_DESCR(HeightSharpenTool::MODE_MEDIUM, "Medium");
    ENUM_ADD_DESCR(HeightSharpenTool::MODE_LOW, "Low");
}

HeightSharpenTool::HeightSharpenTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_sharpen.png"), QString("Sharpen height")),
                         "HeightSharpenTool")
{
}

void HeightSharpenTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eMode>("mode", mode);
}

QWidget* HeightSharpenTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::Value] = "mode";
        builder.RegisterParam<ComboBox>("Mode", fields);
    }
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightSharpenTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::RefPtr<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        DAVA::Vector4 params(strength, 1.0f / static_cast<DAVA::float32>(2 * morphTexture->width), 0.0, 0.0);

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHT_SHARPEN"), 1);
        descr.phaseMaterial->AddFlag(DAVA::FastName("SHARPEN_LEVEL"), static_cast<DAVA::int32>(mode));
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture.Get());
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightSharpenTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params(strength, 1.0f / (2 * floatTexture->width), 0.0f, 0.0f);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);

        FastName levelFlagName("SHARPEN_LEVEL");
        if (phase.phaseMaterial->GetEffectiveFlagValue(levelFlagName) != static_cast<int32>(mode))
        {
            phase.phaseMaterial->SetFlag(levelFlagName, static_cast<int32>(mode));
        }
    }
}

void HeightSharpenTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightSharpenTool)
{
    DAVA::ReflectionRegistrator<HeightSharpenTool>::Begin()[BaseLandscapeTool::SortKey(20)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightSharpenTool::mode)[DAVA::M::EnumT<HeightSharpenTool::eMode>()]
    .End();
}
