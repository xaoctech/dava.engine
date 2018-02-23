#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseLandscapeTool.h"

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Render/Material/NMaterial.h>
#include <Render/Texture.h>

namespace DAVA
{
class Command;
class BaseTextureRenderLandscapeTool : public BaseLandscapeTool
{
public:
    BaseTextureRenderLandscapeTool(LandscapeEditorSystemV2* system)
        : BaseLandscapeTool(system)
    {
    }

    struct BrushPhaseDescriptor
    {
        int32 internalId = std::numeric_limits<int32>::min();
        RefPtr<Texture> renderTarget;
        uint32 renderTargetLevel = 0;
        RefPtr<NMaterial> phaseMaterial;
        FastName passName;
    };

    virtual Vector<BrushPhaseDescriptor> CreateBrushPhases() = 0;
    virtual void PrepareBrushPhase(BrushPhaseDescriptor& phase) const = 0;

    virtual void StoreSnapshots() = 0;
    virtual std::unique_ptr<Command> CreateDiffCommand(const Rect& operationRect) const = 0;
    virtual void OnCommandExecuted(const RECommandNotificationObject& notif) = 0;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(BaseTextureRenderLandscapeTool, BaseLandscapeTool)
    {
        ReflectionRegistrator<BaseTextureRenderLandscapeTool>::Begin()
        .End();
    }
};
} // namespace DAVA