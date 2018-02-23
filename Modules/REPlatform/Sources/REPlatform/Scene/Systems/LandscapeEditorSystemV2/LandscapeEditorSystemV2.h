#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseLandscapeTool.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushRenderHelper.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <TArc/DataProcessing/PropertiesHolder.h>

#include <Base/RefPtr.h>
#include <Entity/SceneSystem.h>
#include <Functional/Function.h>
#include <Render/Highlevel/Landscape.h>

#include <QCursor>

namespace DAVA
{
class UIEvent;
class Texture;
class Command;

class ReadBackRingArray;
class BrushInputController;
class LandscapeEditorSystemV2 : public SceneSystem, public EditorSceneSystem
{
public:
    LandscapeEditorSystemV2(Scene* scene);
    ~LandscapeEditorSystemV2();

    void SetPropertiesCreatorFn(const Function<PropertiesItem(const String& nodeName)>& fn);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Process(float32 delta) override;
    bool Input(UIEvent* uie) override;
    void InputCancelled(UIEvent* uie) override;

    const Vector<BaseLandscapeTool*>& GetAvailableTools() const;
    const BaseLandscapeTool* GetActiveTool() const;
    bool ActivateTool(BaseLandscapeTool* tool);
    void DeactivateTool();

    bool IsEditingAllowed() const;
    void PrepareForEdit(Landscape* landscape);

    int32 GetLandscapeTextureCount(Landscape::eLandscapeTexture textureSemantic) const;
    RefPtr<Texture> GetOriginalLandscapeTexture(Landscape::eLandscapeTexture textureSemantic, int32 index) const;
    void SetOverrideTexture(Landscape::eLandscapeTexture textureSemantic, int32 index, RefPtr<Texture> texture);

    Landscape* GetEditedLandscape() const;

    void UpdateHeightmap(Landscape* landscape, const Vector<uint16>& data, const Rect2i& rect);
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

    void AddDebugDraw(RenderObject* ro);
    void RemoveDebugDraw(RenderObject* ro);

    Scene* GetEditedScene() const;

protected:
    std::unique_ptr<Command> PrepareForSave(bool saveForGame) override;

private:
    BrushInputController* GetActiveInputController() const;

    void BeginOperation();
    void ApplyBrush();
    void EndOperation(bool canceled);

    void StoreSnapshots();
    void CreateDiffCommand();
    void RestoreTextureOverrides();

    void ReadPickTexture(RefPtr<Texture> texture);

    void OnMouseLeaveRenderWidget();
    void OnMouseEnterRenderWidget();

    bool isMouseInRenderWidget = false;

    RefPtr<Texture> currentCursorTexture;

    // operation state flags
    bool operationBeginRequested = false;
    bool operationEndRequested = false;
    bool operationCancelRequested = false;
    bool operationBegan = false;

    Rect operationRect;

    std::unique_ptr<ReadBackRingArray> pickReadBackTextures;
    UVPickTextureCopier uvPickBlitter;

    Set<Entity*> landscapeEntities;
    Landscape* editedLandscape = nullptr;

    Vector<BaseLandscapeTool*> landscapeTools;
    BaseLandscapeTool* activeTool = nullptr;
    BrushInputController* toolInputController = nullptr;
    BaseBrushApplicant* brushApplicant = nullptr;
    std::unique_ptr<BrushInputController> defaultInputController;

    ///////////////////////////////////////
    struct RTMappingKey
    {
        Landscape::eLandscapeTexture semantic;
        int32 index = 0;

        bool operator<(const RTMappingKey& key) const
        {
            if (semantic != key.semantic)
                return semantic < key.semantic;

            return index < key.index;
        }
    };
    Map<RTMappingKey, RefPtr<Texture>> overrideMapping;

    struct BlitWaitingNode
    {
        Token callbackToken;
        Vector<RefPtr<Texture>> texCopies;
    };
    Map<rhi::HSyncObject, BlitWaitingNode> texturesThatWaitingBlit;

    Function<PropertiesItem(const String& nodeName)> propsCreatorFn;
    QCursor prevCursor = QCursor(Qt::ArrowCursor);
};
} // namespace DAVA
