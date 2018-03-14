#pragma once

#include "Asset/Asset.h"
#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Render/Material/Material.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/FXAsset.h"

namespace DAVA
{
class Mesh;
class Scene;
class Entity;
class FilePath;
class Landscape;
class SerializationContext;
class SceneFileConverter
{
public:
    static void ConvertSceneToLevelFormat(Scene* scene, const FilePath& rootFolder);
    static void ConvertRenderComponentsRecursive(Entity* entity, const FilePath& rootFolder);

protected:
    static bool ConvertMesh(Mesh* mesh, Entity* entity, const FilePath& assetsPath);
    static bool ConvertLandscape(Landscape* landscape, Entity* entity, const FilePath& assetsPath);
    static FilePath ConvertDecoration(const FilePath& decorationPath);

    static FilePath FindAssetsFolder(Entity* forEntity);

    static Asset<Material> CreateMaterialAsset(NMaterial* material, const FilePath& pathRoot);
    static void RemoveRedundantProps(NMaterial* material, bool removeUVScaleOffset);
    static void CopyMaterialLocalProps(NMaterial* from, NMaterial* to);
    static bool CompareMaterialLocalProps(NMaterial* material, NMaterial* referenceMaterial);
    static FXDescriptor::eType GetMaterialType(NMaterial* material);
};
};
