#pragma once

#include <Asset/Asset.h>
#include <Base/BaseObject.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <FileSystem/FilePath.h>
#include <Math/AABBox3.h>
#include <Math/Color.h>
#include <Math/Vector.h>

namespace DAVA
{
class Image;
class Heightmap;
class Texture;
class Landscape;
class Entity;
class NMaterial;

class LandscapeProxy : public BaseObject
{
public:
    enum eTilemaskTextures
    {
        TILEMASK_TEXTURE_SOURCE = 0,
        TILEMASK_TEXTURE_DESTINATION,

        TILEMASK_TEXTURE_COUNT
    };

    enum eLandscapeMode
    {
        MODE_CUSTOM_LANDSCAPE = 0,
        MODE_ORIGINAL_LANDSCAPE,

        MODES_COUNT
    };

    static const FastName LANDSCAPE_TEXTURE_TOOL;
    static const FastName LANDSCAPE_TEXTURE_CURSOR; //should use clamp wrap mode
    static const FastName LANSDCAPE_FLAG_CURSOR;
    static const FastName LANSDCAPE_FLAG_TOOL;
    static const FastName LANSDCAPE_FLAG_TOOL_MIX;
    static const FastName LANDSCAPE_PARAM_CURSOR_COORD_SIZE; //x,y - cursor position [0...1] (in landscape space); z,w - cursor size [0...1] (fraction of landscape)

protected:
    virtual ~LandscapeProxy();

public:
    LandscapeProxy(Landscape* landscape, Entity* node);

    void SetMode(LandscapeProxy::eLandscapeMode mode);

    const AABBox3& GetLandscapeBoundingBox();
    Asset<Texture> GetLandscapeTexture(uint32 layerIndex, const FastName& level);
    Color GetLandscapeTileColor(uint32 layerIndex, const FastName& level);
    void SetLandscapeTileColor(uint32 layerIndex, const FastName& level, const Color& color);

    void SetToolTexture(const Asset<Texture>& texture, bool mixColors);

    void SetHeightmap(Heightmap* heightmap);

    void CursorEnable();
    void CursorDisable();
    void SetCursorTexture(const Asset<Texture>& texture);
    void SetCursorSize(float32 size);
    void SetCursorPosition(const Vector2& position);

    Vector3 PlacePoint(const Vector3& point);

    bool IsTilemaskChanged();
    void ResetTilemaskChanged();
    void IncreaseTilemaskChanges();
    void DecreaseTilemaskChanges();

    bool InitTilemaskImageCopy(const Vector<FilePath>& sourceTilemaskPath);
    Image* GetTilemaskImageCopy(uint32 layerIndex);

    void InitTilemaskDrawTextures();
    Asset<Texture> GetTilemaskDrawTexture(uint32 layerIndex, int32 number);
    uint32 GetTilemaskDrawTexturesCount() const;
    void SwapTilemaskDrawTextures();

    Landscape* GetBaseLandscape();
    uint32 GetLayersCount() const;

    void ReloadLayersCountDependentResources();

private:
    void RestoreResources();

protected:
    enum eToolTextureType
    {
        TEXTURE_TYPE_NOT_PASSABLE = 0,
        TEXTURE_TYPE_CUSTOM_COLORS,
        TEXTURE_TYPE_VISIBILITY_CHECK_TOOL,
        TEXTURE_TYPE_RULER_TOOL,

        TEXTURE_TYPES_COUNT
    };

    Vector<Image*> tilemaskImageCopies;
    Vector<Array<Asset<Texture>, TILEMASK_TEXTURE_COUNT>> tilemaskDrawTextures;

    int32 tilemaskWasChanged = 0;

    Landscape* baseLandscape = nullptr;
    NMaterial* landscapeEditorMaterial = nullptr;
    Vector4 cursorCoordSize;

    eLandscapeMode mode = MODE_ORIGINAL_LANDSCAPE;

    Asset<Texture> cursorTexture;
};
} // namespace DAVA