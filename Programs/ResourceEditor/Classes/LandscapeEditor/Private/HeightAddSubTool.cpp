#include "Classes/LandscapeEditor/Private/HeightAddSubTool.h"

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

ENUM_DECLARE(HeightAddSubTool::eBrushMode)
{
    ENUM_ADD_DESCR(HeightAddSubTool::MODE_ADD, "Add");
    ENUM_ADD_DESCR(HeightAddSubTool::MODE_SUB, "Sub");
}

HeightAddSubTool::HeightAddSubTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_add_sub.png"), QString("Add/Sub height")),
                         "HeightAddSubTool")
{
}

void HeightAddSubTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eBrushMode>("mode", mode);
}

QWidget* HeightAddSubTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::Value] = "mode";
        builder.RegisterParam<ComboBox>("Mode (Ctrl)", fields);
    }
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightAddSubTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Asset<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        DAVA::Vector4 params;
        params.x = (GetMode() == MODE_ADD) ? 1.0f : -1.0f;
        params.y = strength;

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHTMAP_ADD_SUB"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), srcHeightTexture);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture1"), morphTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightAddSubTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params;
        params.x = (GetMode() == MODE_ADD) ? 1.0f : -1.0f;
        params.y = strength;
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightAddSubTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
}

HeightAddSubTool::eBrushMode HeightAddSubTool::GetMode() const
{
    bool isPressed = inputController->IsModifierPressed(DAVA::eModifierKeys::CONTROL);
    if (isPressed == false)
    {
        return mode;
    }

    if (mode == MODE_ADD)
    {
        return MODE_SUB;
    }

    return MODE_ADD;
}

void HeightAddSubTool::SetMode(eBrushMode newMode)
{
    mode = newMode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightAddSubTool)
{
    DAVA::ReflectionRegistrator<HeightAddSubTool>::Begin()[BaseLandscapeTool::SortKey(70)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightAddSubTool::GetMode, &HeightAddSubTool::SetMode)[DAVA::M::EnumT<HeightAddSubTool::eBrushMode>()]
    .End();
}
