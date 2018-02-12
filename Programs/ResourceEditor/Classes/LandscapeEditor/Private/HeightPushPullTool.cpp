#include "Classes/LandscapeEditor/Private/HeightPushPullTool.h"

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/KeyboardInputController.h>

#include <TArc/Utils/Utils.h>
#include <TArc/Core/ContextAccessor.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Base/BaseTypes.h>
#include <Base/GlobalEnum.h>
#include <Engine/Engine.h>
#include <Engine/EngineTypes.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <UI/UIEvent.h>

class HeightPushPullTool::PushPullInputController : public DAVA::KeyboardInputController
{
public:
    PushPullInputController(DAVA::float32* delta_, DAVA::float32* strength_)
        : delta(delta_)
        , strength(strength_)
    {
        DVASSERT(delta != nullptr);
        DVASSERT(strength != nullptr);
    }

    void OnInput(DAVA::UIEvent* e) override
    {
        bool sendToBase = true;
        if (e->phase == DAVA::UIEvent::Phase::BEGAN)
        {
            if (e->mouseButton == DAVA::eMouseButtons::LEFT)
            {
                DAVA::GetPrimaryWindow()->SetCursorCapture(DAVA::eCursorCapture::PINNING);
                *delta = 0.0;
                mouseDelta = 0.0;
                sendToBase = false;
            }
        }
        else if (e->phase == DAVA::UIEvent::Phase::ENDED)
        {
            if (e->mouseButton == DAVA::eMouseButtons::LEFT)
            {
                DAVA::GetPrimaryWindow()->SetCursorCapture(DAVA::eCursorCapture::OFF);
                *delta = 0.0;
                mouseDelta = 0.0;
                sendToBase = false;
            }
        }
        else if (e->phase == DAVA::UIEvent::Phase::DRAG)
        {
            if (e->mouseButton == DAVA::eMouseButtons::LEFT)
            {
                DAVA::Vector2 moveDelta = e->point;
                mouseDelta += moveDelta.y;
                DAVA::float32 immStrengedDelta = *strength * 0.01f * mouseDelta;
                *delta = immStrengedDelta;
                sendToBase = false;
            }
        }

        if (sendToBase == true)
        {
            KeyboardInputController::OnInput(e);
        }
    }

    const DAVA::Vector4& GetCurrentCursorUV() const override
    {
        if (IsInOperation())
            return GetBeginOperationCursorUV();

        return KeyboardInputController::GetCurrentCursorUV();
    }

private:
    DAVA::float32 mouseDelta = 0.0;
    DAVA::float32* delta = nullptr;
    DAVA::float32* strength = nullptr;
};

HeightPushPullTool::HeightPushPullTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseHeightEditTool(system, ButtonInfo(DAVA::SharedIcon(":/LandscapeEditor/height_pull_push.png"), QStringLiteral("Push/Pull")),
                         "HeightPushPullTool")
{
    ResetInputController(std::make_unique<PushPullInputController>(&delta, &strength));
}

QWidget* HeightPushPullTool::CreateEditorWidget(const BaseLandscapeTool::WidgetParams& params)
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

DAVA::Vector<DAVA::BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> HeightPushPullTool::CreateBrushPhases()
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
        descr.phaseMaterial->AddFlag(DAVA::FastName("HEIGHTMAP_PUSH_PULL"), 1);
        descr.phaseMaterial->AddTexture(DAVA::FastName("texture0"), srcHeightTexture.Get());
        descr.phaseMaterial->AddProperty(DAVA::FastName("params"), params.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
        descr.renderTarget = floatTexture;

        phases.push_back(descr);
    }
    CreateBasePhases(phases);

    return phases;
}

void HeightPushPullTool::PrepareBrushPhase(BrushPhaseDescriptor& phase) const
{
    using namespace DAVA;
    if (phase.internalId == -1)
    {
        DAVA::Vector4 params;
        params.x = delta;
        phase.phaseMaterial->SetPropertyValue(DAVA::FastName("params"), params.data);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(HeightPushPullTool)
{
    DAVA::ReflectionRegistrator<HeightPushPullTool>::Begin()[BaseLandscapeTool::SortKey(80)]
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .End();
}
