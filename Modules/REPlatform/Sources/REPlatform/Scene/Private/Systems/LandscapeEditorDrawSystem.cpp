#include "REPlatform/Scene/Systems/LandscapeEditorDrawSystem.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/NotPassableTerrainProxy.h"
#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/RulerToolProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Utils/RETextureDescriptorUtils.h"

#include "REPlatform/Commands/InspDynamicModifyCommand.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Global/Constants.h"
#include "REPlatform/Global/StringConstants.h"

#include <Asset/AssetManager.h>
#include <Base/Any.h>
#include <Debug/DVAssert.h>
#include <Engine/EngineContext.h>
#include <Render/Texture.h>
#include <Scene3D/Systems/RenderUpdateSystem.h>

namespace DAVA
{
LandscapeEditorDrawSystem::LandscapeEditorDrawSystem(Scene* scene)
    : SceneSystem(scene, ComponentMask())
{
}

LandscapeEditorDrawSystem::~LandscapeEditorDrawSystem()
{
    SafeRelease(baseLandscape);
    SafeRelease(landscapeProxy);
    SafeRelease(heightmapProxy);
    SafeRelease(customColorsProxy);
    SafeRelease(rulerToolProxy);
    SafeDelete(notPassableTerrainProxy);
}

LandscapeProxy* LandscapeEditorDrawSystem::GetLandscapeProxy()
{
    return landscapeProxy;
}

HeightmapProxy* LandscapeEditorDrawSystem::GetHeightmapProxy()
{
    return heightmapProxy;
}

CustomColorsProxy* LandscapeEditorDrawSystem::GetCustomColorsProxy()
{
    return customColorsProxy;
}

RulerToolProxy* LandscapeEditorDrawSystem::GetRulerToolProxy()
{
    return rulerToolProxy;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableCustomDraw()
{
    if (customDrawRequestCount != 0)
    {
        ++customDrawRequestCount;
        return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    eErrorType initError = Init();
    if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return initError;
    }

    landscapeProxy->SetMode(LandscapeProxy::MODE_CUSTOM_LANDSCAPE);

    ++customDrawRequestCount;

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableCustomDraw()
{
    if (customDrawRequestCount == 0)
    {
        return;
    }

    --customDrawRequestCount;

    if (customDrawRequestCount == 0)
    {
        landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
        UpdateBaseLandscapeHeightmap();
    }
}

bool LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled()
{
    if (!notPassableTerrainProxy)
    {
        return false;
    }

    return notPassableTerrainProxy->IsEnabled();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::IsNotPassableTerrainCanBeEnabled()
{
    return VerifyLandscape();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableNotPassableTerrain()
{
    eErrorType canBeEnabledError = IsNotPassableTerrainCanBeEnabled();
    if (canBeEnabledError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return canBeEnabledError;
    }

    eErrorType enableCustomDrawError = EnableCustomDraw();
    if (enableCustomDrawError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return enableCustomDrawError;
    }

    if (!notPassableTerrainProxy)
    {
        notPassableTerrainProxy = new NotPassableTerrainProxy(baseLandscape->GetHeightmap()->Size());

        DAVA::Rect2i updateRect = DAVA::Rect2i(0, 0, GetHeightmapProxy()->GetHeightmap()->Size(), GetHeightmapProxy()->GetHeightmap()->Size());
        notPassableTerrainProxy->UpdateTexture(heightmapProxy->GetHeightmap(), landscapeProxy->GetLandscapeBoundingBox(), updateRect);
    }

    if (notPassableTerrainProxy->IsEnabled())
    {
        return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
    }

    notPassableTerrainProxy->SetEnabled(true);
    landscapeProxy->SetToolTexture(notPassableTerrainProxy->GetTexture(), false);

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

bool LandscapeEditorDrawSystem::DisableNotPassableTerrain()
{
    if (!notPassableTerrainProxy || !notPassableTerrainProxy->IsEnabled())
    {
        return false;
    }

    notPassableTerrainProxy->SetEnabled(false);
    landscapeProxy->SetToolTexture(nullptr, false);

    DisableCustomDraw();
    return true;
}

void LandscapeEditorDrawSystem::EnableCursor()
{
    landscapeProxy->CursorEnable();
}

void LandscapeEditorDrawSystem::DisableCursor()
{
    landscapeProxy->CursorDisable();
}

void LandscapeEditorDrawSystem::SetCursorTexture(const Asset<Texture>& cursorTexture)
{
    landscapeProxy->SetCursorTexture(cursorTexture);
}

void LandscapeEditorDrawSystem::SetCursorSize(float32 cursorSize)
{
    if (landscapeProxy)
    {
        landscapeProxy->SetCursorSize(cursorSize);
    }
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
    if (landscapeProxy)
    {
        landscapeProxy->SetCursorPosition(cursorPos);
    }
}

void LandscapeEditorDrawSystem::Process(float32 timeElapsed)
{
    if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
    {
        const Rect& changedRect = heightmapProxy->GetChangedRect();
        Rect2i updateRect(static_cast<int32>(changedRect.x), static_cast<int32>(changedRect.y),
                          static_cast<int32>(changedRect.dx), static_cast<int32>(changedRect.dy));

        if (customDrawRequestCount == 0)
        {
            UpdateBaseLandscapeHeightmap();
        }
        else
        {
            if (baseLandscape->IsUpdatable())
            {
                baseLandscape->UpdatePart(updateRect);
            }
            else
            {
                UpdateBaseLandscapeHeightmap();
            }
        }

        if (notPassableTerrainProxy && notPassableTerrainProxy->IsEnabled())
        {
            notPassableTerrainProxy->UpdateTexture(heightmapProxy->GetHeightmap(), landscapeProxy->GetLandscapeBoundingBox(), updateRect);
        }

        heightmapProxy->ResetHeightmapChanged();
    }

    if (customColorsProxy && customColorsProxy->IsTargetChanged())
    {
        customColorsProxy->ResetTargetChanged();
    }
}

void LandscapeEditorDrawSystem::UpdateBaseLandscapeHeightmap()
{
    GetScene()->foliageSystem->SyncFoliageWithLandscape();
}

float32 LandscapeEditorDrawSystem::GetTextureSize(uint32 layerIndex, const FastName& level)
{
    float32 size = 0.f;
    Asset<Texture> texture = baseLandscape->GetPageMaterials(layerIndex, 0)->GetEffectiveTexture(level);
    if (texture)
    {
        size = static_cast<float32>(texture->width);
    }
    return size;
}

Vector3 LandscapeEditorDrawSystem::GetLandscapeSize()
{
    AABBox3 transformedBox;
    baseLandscape->GetBoundingBox().GetTransformedBox(*baseLandscape->GetWorldTransformPtr(), transformedBox);

    Vector3 landSize = transformedBox.max - transformedBox.min;
    return landSize;
}

float32 LandscapeEditorDrawSystem::GetLandscapeMaxHeight()
{
    Vector3 landSize = GetLandscapeSize();
    return landSize.z;
}

Rect LandscapeEditorDrawSystem::GetTextureRect(uint32 layerIndex, const FastName& level)
{
    float32 textureSize = GetTextureSize(layerIndex, level);
    return Rect(Vector2(0.f, 0.f), Vector2(textureSize, textureSize));
}

Rect LandscapeEditorDrawSystem::GetHeightmapRect()
{
    float32 heightmapSize = static_cast<float32>(GetHeightmapProxy()->GetHeightmap()->Size());
    return Rect(Vector2(0.f, 0.f), Vector2(heightmapSize, heightmapSize));
}

Rect LandscapeEditorDrawSystem::GetLandscapeRect()
{
    AABBox3 boundingBox = GetLandscapeProxy()->GetLandscapeBoundingBox();
    Vector2 landPos(boundingBox.min.x, boundingBox.min.y);
    Vector2 landSize((boundingBox.max - boundingBox.min).x,
                     (boundingBox.max - boundingBox.min).y);

    return Rect(landPos, landSize);
}

float32 LandscapeEditorDrawSystem::GetHeightAtHeightmapPoint(const Vector2& point)
{
    Heightmap* heightmap = GetHeightmapProxy()->GetHeightmap();

    int32 hmSize = heightmap->Size();
    int32 x = static_cast<int32>(point.x);
    int32 y = static_cast<int32>(point.y);

    DVASSERT((x >= 0) && (x < hmSize) && (y >= 0) && (y < hmSize),
             "Point must be in heightmap coordinates");

    int nextX = Min(x + 1, hmSize - 1);
    int nextY = Min(y + 1, hmSize - 1);
    int i00 = x + y * hmSize;
    int i01 = nextX + y * hmSize;
    int i10 = x + nextY * hmSize;
    int i11 = nextX + nextY * hmSize;

    const auto hmData = heightmap->Data();
    float h00 = static_cast<float>(hmData[i00]);
    float h01 = static_cast<float>(hmData[i01]);
    float h10 = static_cast<float>(hmData[i10]);
    float h11 = static_cast<float>(hmData[i11]);

    float dx = point.x - static_cast<float>(x);
    float dy = point.y - static_cast<float>(y);
    float h0 = h00 * (1.0f - dx) + h01 * dx;
    float h1 = h10 * (1.0f - dx) + h11 * dx;
    float h = h0 * (1.0f - dy) + h1 * dy;

    return h * GetLandscapeMaxHeight() / static_cast<float32>(Heightmap::MAX_VALUE);
}

float32 LandscapeEditorDrawSystem::GetHeightAtTexturePoint(const FastName& level, const Vector2& point)
{
    auto textureRect = GetTextureRect(0, level);
    if (textureRect.PointInside(point))
    {
        return GetHeightAtHeightmapPoint(TexturePointToHeightmapPoint(level, point));
    }

    return 0.0f;
}

Vector2 LandscapeEditorDrawSystem::HeightmapPointToTexturePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetHeightmapRect(), GetTextureRect(0, level));
}

Vector2 LandscapeEditorDrawSystem::TexturePointToHeightmapPoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetTextureRect(0, level), GetHeightmapRect());
}

Vector2 LandscapeEditorDrawSystem::TexturePointToLandscapePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetTextureRect(0, level), GetLandscapeRect());
}

Vector2 LandscapeEditorDrawSystem::LandscapePointToTexturePoint(const FastName& level, const Vector2& point)
{
    return TranslatePoint(point, GetLandscapeRect(), GetTextureRect(0, level));
}

Vector2 LandscapeEditorDrawSystem::TranslatePoint(const Vector2& point, const Rect& fromRect, const Rect& toRect)
{
    DVASSERT(fromRect.dx != 0.f && fromRect.dy != 0.f);

    Vector2 origRectSize = fromRect.GetSize();
    Vector2 destRectSize = toRect.GetSize();

    Vector2 scale(destRectSize.x / origRectSize.x,
                  destRectSize.y / origRectSize.y);

    Vector2 relPos = point - fromRect.GetPosition();
    Vector2 newRelPos(relPos.x * scale.x,
                      toRect.dy - 1.0f - relPos.y * scale.y);

    Vector2 newPos = newRelPos + toRect.GetPosition();

    return newPos;
}

uint32 LandscapeEditorDrawSystem::GetLayerCount() const
{
    return (baseLandscape == nullptr) ? 0 : baseLandscape->GetLayersCount();
}

KeyedArchive* LandscapeEditorDrawSystem::GetLandscapeCustomProperties()
{
    return GetOrCreateCustomProperties(landscapeNode)->GetArchive();
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::EnableTilemaskEditing()
{
    eErrorType initError = Init();
    if (initError != LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
    {
        return initError;
    }

    landscapeProxy->SetMode(LandscapeProxy::MODE_ORIGINAL_LANDSCAPE);
    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DisableTilemaskEditing()
{
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::Init()
{
    if (heightmapProxy == nullptr)
    {
        Heightmap* heightmap = baseLandscape->GetHeightmap();
        if ((heightmap == nullptr) || (heightmap->Size() == 0))
        {
            return LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
        }
        heightmapProxy = new HeightmapProxy(heightmap);
    }

    if (customColorsProxy == nullptr)
    {
        customColorsProxy = new CustomColorsProxy(Landscape::CUSTOM_COLOR_TEXTURE_SIZE);
    }

    if (rulerToolProxy == nullptr)
    {
        rulerToolProxy = new RulerToolProxy(static_cast<int32>(GetTextureSize(0, Landscape::TEXTURE_TILEMASK)));
    }

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::InitLandscape(Entity* landscapeEntity, Landscape* landscape)
{
    DeinitLandscape();

    if (!landscapeEntity || !landscape)
    {
        return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
    }

    landscapeNode = landscapeEntity;
    baseLandscape = SafeRetain(landscape);

    UpdateTilemaskPathname();

    DVASSERT(landscapeProxy == nullptr);
    landscapeProxy = new LandscapeProxy(baseLandscape, landscapeNode);

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

void LandscapeEditorDrawSystem::DeinitLandscape()
{
    landscapeNode = NULL;
    SafeRelease(landscapeProxy);
    SafeRelease(baseLandscape);
}

void LandscapeEditorDrawSystem::ClampToTexture(const FastName& level, Rect& rect)
{
    GetTextureRect(0, level).ClampToRect(rect);
}

void LandscapeEditorDrawSystem::ClampToHeightmap(Rect& rect)
{
    GetHeightmapRect().ClampToRect(rect);
}

void LandscapeEditorDrawSystem::AddEntity(Entity* entity)
{
    if (IsSystemEnabled() == false)
    {
        return;
    }

    Landscape* landscape = GetLandscape(entity);
    if (landscape != NULL)
    {
        entity->SetLocked(true);

        InitLandscape(entity, landscape);
    }
}

void LandscapeEditorDrawSystem::RemoveEntity(Entity* entity)
{
    if (entity == landscapeNode && IsSystemEnabled())
    {
        SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());

        bool needRemoveBaseLandscape = sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL
                                                                   & ~SceneEditor2::LANDSCAPE_TOOL_TILEMAP_EDITOR);

        sceneEditor->DisableToolsInstantly(SceneEditor2::LANDSCAPE_TOOLS_ALL);

        if (needRemoveBaseLandscape)
        {
            sceneEditor->renderUpdateSystem->RemoveEntity(entity);
        }

        DeinitLandscape();

        Entity* landEntity = FindLandscapeEntity(sceneEditor);
        if (landEntity != nullptr && landEntity != entity)
        {
            InitLandscape(landEntity, GetLandscape(landEntity));
        }
    }
}

void LandscapeEditorDrawSystem::PrepareForRemove()
{
    DeinitLandscape();
}

bool LandscapeEditorDrawSystem::SaveTileMaskTexture()
{
    if (baseLandscape == nullptr || !GetLandscapeProxy()->IsTilemaskChanged())
    {
        return false;
    }

    bool shouldRestore = false;
    for (uint32 i = 0; i < GetLayerCount(); ++i)
    {
        Asset<Texture> texture = GetTileMaskTexture(i);
        if (texture != nullptr)
        {
            Image* image = texture->CreateImageFromRegion();

            if (image)
            {
                ImageSystem::Save(sourceTilemaskPath[i], image);
                SafeRelease(image);
            }

            GetLandscapeProxy()->ResetTilemaskChanged();
            shouldRestore = true;
        }
    }

    return shouldRestore;
}

void LandscapeEditorDrawSystem::ResetTileMaskTextures()
{
    if (baseLandscape == nullptr)
    {
        return;
    }

    AssetManager* assetManager = GetEngineContext()->assetManager;
    for (uint32 i = 0; i < GetLayerCount(); ++i)
    {
        if (i < uint32(sourceTilemaskPath.size()))
        {
            Texture::PathKey key(sourceTilemaskPath[i]);
            Asset<Texture> texture = assetManager->GetAsset<Texture>(key, AssetManager::SYNC);
            // GFX_COMPLETE - WTF???
            //texture->Reload();
            SetTileMaskTexture(i, texture);
        }
    }
}

void LandscapeEditorDrawSystem::SetTileMaskTexture(uint32 layerIndex, const Asset<Texture>& texture)
{
    if (baseLandscape == nullptr)
    {
        return;
    }

    for (uint32 i = 0; i < baseLandscape->GetPageMaterialCount(layerIndex); ++i)
    {
        NMaterial* landscapeMaterial = baseLandscape->GetPageMaterials(layerIndex, i);
        while (landscapeMaterial != nullptr)
        {
            if (landscapeMaterial->HasLocalTexture(Landscape::TEXTURE_TILEMASK))
                break;

            landscapeMaterial = landscapeMaterial->GetParent();
        }

        if (landscapeMaterial != nullptr)
        {
            landscapeMaterial->SetTexture(Landscape::TEXTURE_TILEMASK, texture);
        }
    }
}

Asset<Texture> LandscapeEditorDrawSystem::GetTileMaskTexture(uint32 layerIndex)
{
    if (baseLandscape != nullptr)
    {
        NMaterial* landscapeMaterial = baseLandscape->GetPageMaterials(layerIndex, 0);
        if (landscapeMaterial != nullptr)
        {
            return landscapeMaterial->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
        }
    }

    return nullptr;
}

LandscapeEditorDrawSystem::eErrorType LandscapeEditorDrawSystem::VerifyLandscape() const
{
    using namespace DAVA;

    //landscape initialization should be handled by AddEntity/RemoveEntity methods
    if (!landscapeNode || !baseLandscape || !landscapeProxy)
    {
        return LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
    }

    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        Asset<Texture> tileMask = landscapeProxy->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK);
        if (tileMask == nullptr || tileMask->IsPinkPlaceholder())
        {
            return LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT;
        }
    }

    Asset<Texture> texColor = baseLandscape->GetPageMaterials(0, 0)->GetEffectiveTexture(Landscape::TEXTURE_COLOR);
    if ((texColor == nullptr || texColor->IsPinkPlaceholder()))
    {
        return LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT;
    }

    Heightmap* heightmap = baseLandscape->GetHeightmap();
    if ((heightmap == nullptr) || (heightmap->Size() == 0))
    {
        return LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
    }

    return LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
}

Landscape* LandscapeEditorDrawSystem::GetBaseLandscape() const
{
    return baseLandscape;
}

String LandscapeEditorDrawSystem::GetDescriptionByError(eErrorType error)
{
    String ret;
    switch (error)
    {
    case LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_LANDSCAPE_ENTITY_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURE_ABSETN;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_FULLTILED_TEXTURE_ABSETN;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILE_TEXTURE_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_COLOR_TEXTURE_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_HEIGHTMAP_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_CUSTOMCOLORS_ABSENT;
        break;
    case LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURES_DIMENSIONS_DOES_NOT_MATCH:
        ret = ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_TILEMASK_TEXTURES_DIMENSIONS_DOES_NOT_MATCH;
        break;

    default:
        DVASSERT(false && "Unknown error");
        break;
    }
    return ret;
}

void LandscapeEditorDrawSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    static const FastName heightmapPath("heightmapPath");

    commandNotification.ForEach<InspDynamicModifyCommand>([this](const InspDynamicModifyCommand* cmd) {
        if (Landscape::TEXTURE_TILEMASK == cmd->key)
        {
            UpdateTilemaskPathname();
        }
    });

    commandNotification.ForEach<SetFieldValueCommand>([this](const SetFieldValueCommand* cmd) {
        if (heightmapPath == cmd->GetField().key.Cast<FastName>(FastName("")) && baseLandscape != nullptr)
        {
            Heightmap* heightmap = baseLandscape->GetHeightmap();
            if ((heightmap != nullptr) && (heightmap->Size() > 0))
            {
                ScopedPtr<Heightmap> clonedHeightmap(heightmap->Clone(nullptr));
                SafeRelease(heightmapProxy);
                heightmapProxy = new HeightmapProxy(clonedHeightmap);

                float32 size = static_cast<float32>(heightmapProxy->GetHeightmap()->Size());
                heightmapProxy->UpdateRect(Rect(0.f, 0.f, size, size));
            }
        }
    });
}

void LandscapeEditorDrawSystem::UpdateTilemaskPathname()
{
    int32 pcount = 0;
    if (nullptr != baseLandscape)
    {
        for (uint32 i = 0; i < baseLandscape->GetLayersCount(); ++i)
        {
            auto texture = baseLandscape->GetPageMaterials(i, 0)->GetEffectiveTexture(Landscape::TEXTURE_TILEMASK);
            if (nullptr != texture)
            {
                FilePath path = texture->GetDescriptor()->GetSourceTexturePathname();
                if (path.GetType() == FilePath::PATH_IN_FILESYSTEM)
                {
                    if (pcount < sourceTilemaskPath.size())
                        sourceTilemaskPath[i] = path;
                    else
                        sourceTilemaskPath.push_back(path);
                }
                ++pcount;
            }
        }
    }
}

bool LandscapeEditorDrawSystem::InitTilemaskImageCopy()
{
    DVASSERT(landscapeProxy != nullptr);
    return landscapeProxy->InitTilemaskImageCopy(sourceTilemaskPath);
}
} // namespace DAVA
