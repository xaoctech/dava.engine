#include "Classes/LandscapeEditor/Private/HeightCloneStampTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/KeyboardInputController.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Slider.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Asset/AssetManager.h>
#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Input/InputElements.h>
#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/Texture.h>
#include <UI/UIEvent.h>

namespace HeightCloneStampToolDetail
{
using namespace DAVA;
class CloneStampInputController : public DAVA::KeyboardInputController
{
public:
    void OnInput(UIEvent* e)
    {
        if (e->phase == UIEvent::Phase::KEY_DOWN &&
            (e->key == eInputElements::KB_LCTRL || e->key == eInputElements::KB_RCTRL))
        {
            mouseMoved = false;
            clonePos = GetCurrentCursorUV();
            ctrlPressed = true;
        }

        if (e->phase == UIEvent::Phase::KEY_UP &&
            (e->key == eInputElements::KB_LCTRL || e->key == eInputElements::KB_RCTRL)
            && ctrlPressed == true)
        {
            ctrlPressed = false;

            Vector4 currentPos = GetCurrentCursorUV();
            clonePos = Vector4(clonePos.x - currentPos.x, clonePos.y - currentPos.y, 0.0, clonePos.w);
            if (mouseMoved == false)
            {
                clonePos.w = 1.0;
            }
        }

        if (e->phase == UIEvent::Phase::MOVE || e->phase == UIEvent::Phase::DRAG)
        {
            mouseMoved = true;
        }

        KeyboardInputController::OnInput(e);
    }

    Vector4 GetClonePos() const
    {
        if (ctrlPressed == true)
        {
            return clonePos;
        }

        Vector4 currentPos = GetCurrentCursorUV();
        if (clonePos.w < 1.0)
        {
            return Vector4(currentPos.x + clonePos.x, currentPos.y + clonePos.y, 0.0, clonePos.w);
        }

        return currentPos;
    }

    Vector2 GetCloneOffset() const
    {
        if (clonePos.w < 1.0)
        {
            return Vector2(clonePos.x, clonePos.y);
        }

        return Vector2(0.0, 0.0);
    }

private:
    bool mouseMoved = false;
    bool ctrlPressed = false;
    Vector4 clonePos;
};
} // namespace HeightCloneStampToolDetail

HeightCloneStampTool::HeightCloneStampTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/clone.png"), QString("Clone height")),
                         "HeightCloneStampTool")
{
    ResetInputController(std::make_unique<HeightCloneStampToolDetail::CloneStampInputController>());
}

void HeightCloneStampTool::Activate(const DAVA::PropertiesItem& settings)
{
    BaseHeightEditTool::Activate(settings);

    DAVA::Asset<DAVA::Texture> heightTexture = system->GetOriginalLandscapeTexture(DAVA::Landscape::HEIGHTMAP_TEXTURE, 0);

    DAVA::Texture::RenderTargetTextureKey descr;
    descr.height = heightTexture->width;
    descr.width = heightTexture->width;
    descr.format = DAVA::FORMAT_RGBA8888;
    descr.mipLevelsCount = 1;
    descr.isDepth = false;
    descr.needPixelReadback = false;
    descr.textureType = rhi::TEXTURE_TYPE_2D;

    coverTexture = DAVA::GetEngineContext()->assetManager->GetAsset<DAVA::Texture>(descr, DAVA::AssetManager::SYNC);
    coverTexture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    coverTexture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    coverTextureGenerateMaterial.ConstructInplace();
    coverTextureGenerateMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
    coverTextureGenerateMaterial->AddFlag(DAVA::FastName("RENDER_BRUSH_FORM"), 1);

    DAVA::Color redColor = DAVA::Color::Red;
    DAVA::Vector4 pos(0.0, 0.0, 0.0, 0.0);
    DAVA::Vector2 rotation(0.0f, 1.0f);
    DAVA::float32 invFactor = 1.0f;

    coverTextureGenerateMaterial->AddProperty(DAVA::FastName("params"), redColor.color, rhi::ShaderProp::TYPE_FLOAT4);
    coverTextureGenerateMaterial->AddProperty(DAVA::FastName("landCursorPosition"), pos.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
    coverTextureGenerateMaterial->AddProperty(DAVA::FastName("cursorRotation"), rotation.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
    coverTextureGenerateMaterial->AddProperty(DAVA::FastName("invertFactor"), &invFactor, rhi::ShaderProp::TYPE_FLOAT1, 1);
    coverTextureGenerateMaterial->AddTexture(DAVA::FastName("landCursorTexture"), GetCursorTexture());
}

DAVA::Asset<DAVA::Texture> HeightCloneStampTool::GetCustomCoverTexture() const
{
    return coverTexture;
}

void HeightCloneStampTool::Process(DAVA::float32 delta)
{
    HeightCloneStampToolDetail::CloneStampInputController* controller = static_cast<HeightCloneStampToolDetail::CloneStampInputController*>(inputController.get());

    DAVA::Vector4 clonePos = controller->GetClonePos();
    if (clonePos.w < 1.0)
    {
        DAVA::Vector2 brushSize = GetBrushSize();
        clonePos.z = brushSize.x;
        clonePos.w = brushSize.y;
    }
    else
    {
        clonePos.x = clonePos.y = 10.0f;
        clonePos.z = clonePos.w = 0.0;
    }

    DAVA::TextureBlitter::TargetInfo info;
    info.renderTarget = coverTexture;
    info.textureLevel = 0;
    coverTextureGenerateMaterial->SetPropertyValue(DAVA::FastName("landCursorPosition"), clonePos.data);
    coverTextureGenerateMaterial->SetTexture(DAVA::FastName("landCursorTexture"), GetCursorTexture());
    if (coverTextureGenerateMaterial->PreBuildMaterial(DAVA::PASS_FORWARD) == false)
    {
        return;
    }

    blitter.BlitTexture(info, coverTextureGenerateMaterial, DAVA::PRIORITY_SERVICE_2D);
}

QWidget* HeightCloneStampTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
{
    using namespace DAVA;

    DAVA::Reflection model = DAVA::Reflection::Create(DAVA::ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    CreateBrushSelector(builder);
    CreateBrushSizeControl(builder);

    return builder.GetWidget();
}

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightCloneStampTool::CreateBrushPhases()
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
        params.x = 0.0;
        params.y = 0.0;

        BrushPhaseDescriptor descr;
        descr.internalId = -1;
        descr.passName = DAVA::PASS_FORWARD;
        descr.phaseMaterial.Set(new DAVA::NMaterial());
        descr.phaseMaterial->SetFXName(DAVA::NMaterialName::LANDSCAPE_BRUSH);
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHT_CLONE"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture1"), srcHeightTexture);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), morphTexture);
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightCloneStampTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        HeightCloneStampToolDetail::CloneStampInputController* controller = static_cast<HeightCloneStampToolDetail::CloneStampInputController*>(inputController.get());
        DAVA::Vector2 cloneOffset = controller->GetCloneOffset();
        DAVA::Vector4 params;
        params.x = cloneOffset.x;
        params.y = -cloneOffset.y;
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

void HeightCloneStampTool::Deactivate(DAVA::PropertiesItem& settings)
{
    coverTexture.reset();
    coverTextureGenerateMaterial = DAVA::RefPtr<DAVA::NMaterial>();
    BaseHeightEditTool::Deactivate(settings);
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightCloneStampTool)
{
    DAVA::ReflectionRegistrator<HeightCloneStampTool>::Begin()[BaseLandscapeTool::SortKey(9)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .End();
}
