#include "Classes/LandscapeEditor/Private/HeightRaiseLowerTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Base/GlobalEnum.h>
#include <Engine/EngineTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderPassNames.h>

ENUM_DECLARE(HeightRaiseLowerTool::eBrushMode)
{
    ENUM_ADD_DESCR(HeightRaiseLowerTool::MODE_RISE, "Rise");
    ENUM_ADD_DESCR(HeightRaiseLowerTool::MODE_LOWER, "Lower");
}

HeightRaiseLowerTool::HeightRaiseLowerTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_raise_lower.png"), QStringLiteral("Rise/Lower")),
                         "HeightRaiseLowerTool")
{
}

void HeightRaiseLowerTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eBrushMode>("mode", mode);
}

QWidget* HeightRaiseLowerTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
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

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightRaiseLowerTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Vector4 params;
        params.x = (GetMode() == MODE_RISE) ? 1.0f : -1.0f;
        params.y = strength;

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHTMAP_LOWER_RISE"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightRaiseLowerTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params;
        params.x = (GetMode() == MODE_RISE) ? 1.0f : -1.0f;
        params.y = strength;
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightRaiseLowerTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
}

HeightRaiseLowerTool::eBrushMode HeightRaiseLowerTool::GetMode() const
{
    bool isPressed = inputController->IsModifierPressed(DAVA::eModifierKeys::CONTROL);
    if (isPressed == false)
    {
        return mode;
    }

    if (mode == MODE_RISE)
    {
        return MODE_LOWER;
    }

    return MODE_RISE;
}

void HeightRaiseLowerTool::SetMode(eBrushMode newMode)
{
    mode = newMode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightRaiseLowerTool)
{
    DAVA::ReflectionRegistrator<HeightRaiseLowerTool>::Begin()[BaseLandscapeTool::SortKey(90)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightRaiseLowerTool::GetMode, &HeightRaiseLowerTool::SetMode)[DAVA::M::EnumT<HeightRaiseLowerTool::eBrushMode>()]
    .End();
}
