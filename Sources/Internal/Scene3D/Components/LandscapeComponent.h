#pragma once

#include "Asset/Asset.h"
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"
#include "Render/Material/Material.h"

namespace DAVA
{
class Entity;
class Landscape;
class KeyedArchive;
class SerializationContext;
class LandscapeComponent : public Component
{
protected:
    virtual ~LandscapeComponent();

public:
    LandscapeComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    Landscape* GetLandscape() const;

    void SetLayersCount(uint32 count);
    uint32 GetLayersCount() const;

    void SetPageMaterialPath(uint32 layer, uint32 lod, const FilePath& path);
    const FilePath& GetPageMaterialPath(uint32 layer, uint32 lod) const;

    void SetHeightmapPath(const FilePath& path);
    const FilePath& GetHeighmapPath() const;

    void SetLandscapeMaterialPath(const FilePath& path);
    const FilePath& GetLandscapeMaterialPath() const;

    void SetLandscapeSize(float32 size);
    float32 GetLandscapeSize() const;

    void SetLandscapeHeight(float32 height);
    float32 GetLandscapeHeight() const;

    void SetMiddleLODLevel(uint32 level);
    uint32 GetMiddleLODLevel() const;

    void SetMacroLODLevel(uint32 level);
    uint32 GetMacroLODLevel() const;

    void SetMaxTexturingLevel(uint32 level);
    uint32 GetMaxTexturingLevel() const;

    void SetTessellationLevelCount(uint32 levelCount);
    uint32 GetTessellationLevelCount() const;

    void SetTessellationHeight(float32 height);
    float32 GetTessellationHeight() const;

private:
    using PageMaterials = Array<Asset<Material>, 3>;

    Asset<Material> landscapeMaterial = nullptr;

    FilePath heightmapPath;

    float32 landscapeHeight = 0.f;
    float32 landscapeSize = 0.f;

    uint32 middleLODLevel = 0u;
    uint32 macroLODLevel = 0u;
    uint32 maxTexturingLevel = 10;
    uint32 tessellationLevelCount = 3;
    float32 tessellationHeight = 0.4f;

    Vector<PageMaterials> layersPageMaterials;

    //runtime
    Landscape* landscape = nullptr;

    DAVA_VIRTUAL_REFLECTION(LandscapeComponent, Component);
};
}
