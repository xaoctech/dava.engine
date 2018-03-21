#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystem.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Render/UniqueStateSet.h>

namespace DAVA
{
class TilemaskEditorSystem : public LandscapeEditorSystem
{
public:
    enum eTilemaskDrawType
    {
        TILEMASK_DRAW_NORMAL = 0,
        TILEMASK_DRAW_COPY_PASTE,

        TILEMASK_DRAW_TYPES_COUNT
    };

    TilemaskEditorSystem(Scene* scene);
    virtual ~TilemaskEditorSystem();

    LandscapeEditorDrawSystem::eErrorType EnableLandscapeEditing();
    bool DisableLandscapeEdititing();

    void Draw() override;
    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;
    void InputCancelled(UIEvent* event) override;

    void PrepareForRemove() override
    {
    }

    void SetBrushSize(int32 brushSize);
    int32 GetBrushSize();
    void SetStrength(float32 strength);
    float32 GetStrength();
    void SetToolImage(const FilePath& toolImagePath, int32 index);
    int32 GetToolImage();
    void SetTileTexture(uint32 tileTexture);
    uint32 GetTileTextureIndex();
    uint32 GetTileTextureCount() const;

    void SetDrawingType(eTilemaskDrawType type);
    eTilemaskDrawType GetDrawingType();

protected:
    uint32 curToolSize;

    Asset<Texture> toolImageTexture;
    Asset<Texture> landscapeTilemaskTexture;

    uint32 tileTextureNum;

    NMaterial* editorMaterial;

    eTilemaskDrawType drawingType;
    eTilemaskDrawType activeDrawingType;
    float32 strength;
    float32 strengthSign;
    FilePath toolImagePath;
    int32 toolImageIndex;

    rhi::Packet quadPacket;
    uint32 quadVertexLayoutID;

    Vector2 copyPasteFrom;
    Vector2 copyPasteOffset;
    Vector<Vector4> drawMasks;

    Rect updatedRectAccumulator;

    uint32 previousLayerCount = 0;

    bool editingIsEnabled;

    Asset<Texture> toolTexture;
    bool toolSpriteUpdated;

    bool needCreateUndo;

    const FastName& textureLevel;

    void UpdateBrushTool();
    void UpdateToolImage();

    void AddRectToAccumulator(const Rect& rect);
    void ResetAccumulatorRect();
    Rect GetUpdatedRect();

    void CreateMaskTexture();

    void CreateUndoPoint();

    void InitSprites();

    void FinishEditing();
};
} // namespace DAVA
