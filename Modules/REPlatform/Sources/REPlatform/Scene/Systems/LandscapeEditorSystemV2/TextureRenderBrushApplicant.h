#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseBrushApplicant.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseTextureRenderLandscapeTool.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushRenderHelper.h"

namespace DAVA
{
class TextureRenderBrushApplicant : public BaseBrushApplicant
{
public:
    TextureRenderBrushApplicant(BaseTextureRenderLandscapeTool* tool);

    void StoreSnapshots() override;
    void ApplyBrush(Scene* scene, const Rect& applyRect) override;
    std::unique_ptr<Command> CreateDiffCommand(const Rect& operationRect) override;

    void OnCommandExecuted(const RECommandNotificationObject& notif) override;

    static const char* COVER_TEXTURE_SLOT_NAME;
    static const char* CURSOR_TEXTURE_SLOT_NAME;
    static const char* CURSOR_POS_PROP_NAME;
    static const char* CURSOR_ROTATION_PROP_NAME;
    static const char* INVERTION_FACTOR_PROP_NAME;

private:
    BaseTextureRenderLandscapeTool* tool;
    Vector<BaseTextureRenderLandscapeTool::BrushPhaseDescriptor> phases;
    BrushApplyHelper brushApplyHelper;
    bool phasesCreated = false;
};
} // namespace DAVA