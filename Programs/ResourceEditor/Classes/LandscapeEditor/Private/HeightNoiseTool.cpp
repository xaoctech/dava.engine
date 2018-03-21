#include "Classes/LandscapeEditor/Private/HeightNoiseTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h>
#include <REPlatform/Scene/Utils/Utils.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Slider.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Utils/Random.h>

ENUM_DECLARE(HeightNoiseTool::eMode)
{
    ENUM_ADD_DESCR(HeightNoiseTool::MODE_UNSIGNED, "Unsigned");
    ENUM_ADD_DESCR(HeightNoiseTool::MODE_SIGNED, "Signed");
}

HeightNoiseTool::HeightNoiseTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_noise.png"), QString("Noise")),
                         "HeightNoiseTool")
{
    inputController->RegisterVarCallback(DAVA::eInputElements::KB_K, [this](const DAVA::Vector2& delta) {
        kernelSize = DAVA::Saturate(kernelSize + delta.y * 0.1);
    });

    noiseTexture = DAVA::CreateSingleMipTexture(DAVA::FilePath("~res:/ResourceEditor/LandscapeEditorV2/Resources/noise_1024x1024.png"));
    noiseTexture->SetWrapMode(rhi::TEXADDR_WRAP, rhi::TEXADDR_WRAP);
    noiseTexture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
}

HeightNoiseTool::~HeightNoiseTool()
{
    noiseTexture.reset();
    int x = 0;
    x++;
}

void HeightNoiseTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    mode = toolSettings.Get<eMode>("mode", mode);
    kernelSize = toolSettings.Get<DAVA::float32>("kernelSize", kernelSize);
}

QWidget* HeightNoiseTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
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
    {
        ControlDescriptorBuilder<Slider::Fields> fields;
        fields[Slider::Fields::Value] = "kernelSize";
        builder.RegisterParam<Slider>("Kernel size (K)", fields);
    }
    CreateStrengthSlider(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightNoiseTool::CreateBrushPhases()
{
    DAVA::Vector<BrushPhaseDescriptor> phases;
    if (system->GetLandscapeTextureCount(DAVA::Landscape::HEIGHTMAP_TEXTURE) != 1)
    {
        return phases;
    }

    InitTextures();
    {
        DAVA::Asset<DAVA::Texture> srcHeightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);

        DAVA::Vector4 params = GetParams();
        DAVA::Vector4 random = GetRandom();

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHT_NOISE"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture1"), noiseTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.phaseMaterial->AddProperty(DAVA::FastName("random"), random.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightNoiseTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params = GetParams();
        DAVA::Vector4 random = GetRandom();

        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("random"), random.data);
    }
}

void HeightNoiseTool::Deactivate(DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Deactivate(settings);
    DAVA::PropertiesItem toolSettings = settings.CreateSubHolder(toolName);
    toolSettings.Set("mode", mode);
    toolSettings.Set("kernelSize", kernelSize);
}

DAVA::Vector4 HeightNoiseTool::GetParams() const
{
    return DAVA::Vector4(strength, kernelSize, static_cast<DAVA::float32>(floatTexture->width), static_cast<DAVA::float32>(GetMode()));
}

DAVA::Vector4 HeightNoiseTool::GetRandom() const
{
    DAVA::Random* random = DAVA::GetEngineContext()->random;
    DAVA::float32 randomX = random->RandFloat32InBounds(-1.0f, 1.0f);
    DAVA::float32 randomY = random->RandFloat32InBounds(-1.0f, 1.0f);
    DAVA::float32 randomZ = random->RandFloat32InBounds(-1.0f, 1.0f);

    DAVA::float32 randomSin;
    DAVA::float32 randomCos;

    DAVA::SinCosFast(randomZ, randomSin, randomCos);
    return DAVA::Vector4(randomX, randomY, randomSin, randomCos);
}

HeightNoiseTool::eMode HeightNoiseTool::GetMode() const
{
    bool isPressed = inputController->IsModifierPressed(DAVA::eModifierKeys::CONTROL);
    if (isPressed == false)
    {
        return mode;
    }

    if (mode == MODE_SIGNED)
    {
        return MODE_UNSIGNED;
    }

    return MODE_SIGNED;
}

void HeightNoiseTool::SetMode(HeightNoiseTool::eMode newMode)
{
    mode = newMode;
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightNoiseTool)
{
    DAVA::ReflectionRegistrator<HeightNoiseTool>::Begin()[BaseLandscapeTool::SortKey(10)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Field("mode", &HeightNoiseTool::GetMode, &HeightNoiseTool::SetMode)[DAVA::M::EnumT<HeightNoiseTool::eMode>()]
    .Field("kernelSize", &HeightNoiseTool::kernelSize)[DAVA::M::Range(0.0f, 1.0f, 0.001f)]
    .End();
}
