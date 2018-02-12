#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/TextureRenderBrushApplicant.h"

#include <Scene3D/Scene.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Command/Command.h>

namespace DAVA
{
const char* TextureRenderBrushApplicant::COVER_TEXTURE_SLOT_NAME = "landCoverTexture";
const char* TextureRenderBrushApplicant::CURSOR_TEXTURE_SLOT_NAME = "landCursorTexture";
const char* TextureRenderBrushApplicant::CURSOR_POS_PROP_NAME = "landCursorPosition";
const char* TextureRenderBrushApplicant::CURSOR_ROTATION_PROP_NAME = "cursorRotation";
const char* TextureRenderBrushApplicant::INVERTION_FACTOR_PROP_NAME = "invertFactor";

TextureRenderBrushApplicant::TextureRenderBrushApplicant(BaseTextureRenderLandscapeTool* tool_)
    : tool(tool_)
{
    DVASSERT(tool != nullptr);
    phases = tool->CreateBrushPhases();
}

void TextureRenderBrushApplicant::StoreSnapshots()
{
    tool->StoreSnapshots();
}

void TextureRenderBrushApplicant::ApplyBrush(Scene* scene, const Rect& applyRect)
{
    FastName cursorTextureSlotName(CURSOR_TEXTURE_SLOT_NAME);
    FastName cursorPositionPropName(CURSOR_POS_PROP_NAME);
    FastName cursorRotationPropName(CURSOR_ROTATION_PROP_NAME);
    FastName invertionPropName(INVERTION_FACTOR_PROP_NAME);

    int32 basePriority = scene->renderSystem->GetConfiguration().basePriority;

    for (BaseTextureRenderLandscapeTool::BrushPhaseDescriptor& descr : phases)
    {
        if (phasesCreated == false)
        {
            descr.phaseMaterial->AddProperty(cursorPositionPropName, cursorUV.data, rhi::ShaderProp::TYPE_FLOAT4, 1);
            descr.phaseMaterial->AddProperty(cursorRotationPropName, cursorRotarion.data, rhi::ShaderProp::TYPE_FLOAT2, 1);
            descr.phaseMaterial->AddProperty(invertionPropName, &invertionFactor, rhi::ShaderProp::TYPE_FLOAT1, 1);
            descr.phaseMaterial->AddTexture(cursorTextureSlotName, cursorTexture.Get());
        }
        else
        {
            descr.phaseMaterial->SetPropertyValue(cursorPositionPropName, cursorUV.data);
            descr.phaseMaterial->SetPropertyValue(cursorRotationPropName, cursorRotarion.data);
            descr.phaseMaterial->SetPropertyValue(invertionPropName, &invertionFactor);
            descr.phaseMaterial->SetTexture(cursorTextureSlotName, cursorTexture.Get());
        }

        tool->PrepareBrushPhase(descr);
        brushApplyHelper.ApplyBrush(descr, applyRect, basePriority);
    }

    phasesCreated = true;
}

std::unique_ptr<Command> TextureRenderBrushApplicant::CreateDiffCommand(const Rect& operationRect)
{
    return tool->CreateDiffCommand(operationRect);
}

void TextureRenderBrushApplicant::OnCommandExecuted(const RECommandNotificationObject& notif)
{
    tool->OnCommandExecuted(notif);
}

} // namespace DAVA
