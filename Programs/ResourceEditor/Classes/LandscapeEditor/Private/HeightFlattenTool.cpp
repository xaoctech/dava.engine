#include "Classes/LandscapeEditor/Private/HeightFlattenTool.h"

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

ENUM_DECLARE(HeightFlattenTool::eBrushMode)
{
    ENUM_ADD_DESCR(HeightFlattenTool::MODE_STATIC, "Static");
    ENUM_ADD_DESCR(HeightFlattenTool::MODE_DYNAMIC, "Dynamic");
}

HeightFlattenTool::HeightFlattenTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_flatten.png"), QString("Flatten height")),
                         "HeightFlattenTool")
{
}

void HeightFlattenTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eBrushMode>("mode", mode);
}

QWidget* HeightFlattenTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    {
        ControlDescriptorBuilder<ComboBox::Fields> fields;
        fields[ComboBox::Fields::Value] = "mode";
        builder.RegisterParam<ComboBox>("Brush mode (Ctrl)", fields);
    }
    CreateRotationSlider(builder);
    CreateBrushSizeControl(builder);
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightFlattenTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Asset<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);
        DAVA::Vector4 params(0.0, 0.0, 0.0, 0.0);

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHTMAP_FLATTEN"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture1"), srcHeightTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightFlattenTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 startOpUV = GetMode() == MODE_STATIC ? inputController->GetBeginOperationCursorUV() : inputController->GetCurrentCursorUV();
        DAVA::Vector4 params(startOpUV.x, 1.0f - startOpUV.y, strength, 0.0f);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightFlattenTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
}

HeightFlattenTool::eBrushMode HeightFlattenTool::GetMode() const
{
    bool isPressed = inputController->IsModifierPressed(DAVA::eModifierKeys::CONTROL);
    if (isPressed == false)
    {
        return mode;
    }

    if (mode == MODE_STATIC)
    {
        return MODE_DYNAMIC;
    }

    return MODE_STATIC;
}

void HeightFlattenTool::SetMode(eBrushMode newMode)
{
    mode = newMode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightFlattenTool)
{
    DAVA::ReflectionRegistrator<HeightFlattenTool>::Begin()[BaseLandscapeTool::SortKey(50)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightFlattenTool::GetMode, &HeightFlattenTool::SetMode)[DAVA::M::EnumT<HeightFlattenTool::eBrushMode>()]
    .End();
}
