#include "FBXMaterialImport.h"

#include "Asset/Asset.h"
#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "FileSystem/FilePath.h"
#include "Render/Material/Material.h"
#include "Render/Material/FXAsset.h"
#include "Render/Material/NMaterial.h"
#include "Scene3D/AssetLoaders/MaterialAssetLoader.h"

namespace DAVA
{
namespace FBXImporterDetails
{
namespace FBXMaterialImportDetails
{
Map<std::pair<const FbxSurfaceMaterial*, uint32>, Asset<Material>> materialCache;
FilePath currentAssetsFolder;
}

void SetMaterialAssetsFolder(const FilePath& filepath)
{
    DVASSERT(filepath.IsDirectoryPathname());
    FBXMaterialImportDetails::currentAssetsFolder = filepath;
}

FilePath ImportMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence)
{
    using namespace FBXMaterialImportDetails;

    auto found = materialCache.find(std::make_pair(fbxMaterial, maxVertexInfluence));
    if (found == materialCache.end())
    {
        const char* materialName = "UNNAMED";

        NMaterial* material = new NMaterial(FXDescriptor::TYPE_COMMON);
        material->SetFXName(NMaterialName::FORWARD_PBS);

        if (maxVertexInfluence > 0)
        {
            if (maxVertexInfluence == 1)
                material->AddFlag(NMaterialFlagName::FLAG_HARD_SKINNING, 1);
            else
                material->AddFlag(NMaterialFlagName::FLAG_SOFT_SKINNING, maxVertexInfluence);
        }

        if (fbxMaterial != nullptr)
        {
            materialName = fbxMaterial->GetName();

            Vector<std::pair<FbxLayerElement::EType, FastName>> texturesToImport = {
                { FbxLayerElement::eTextureDiffuse, NMaterialTextureName::TEXTURE_ALBEDO },
                { FbxLayerElement::eTextureNormalMap, NMaterialTextureName::TEXTURE_NORMAL }
            };

            for (auto& tex : texturesToImport)
            {
                FilePath texturePath = GetFBXTexturePath(fbxMaterial, tex.first);
                if (!texturePath.IsEmpty())
                    material->AddTexture(tex.second, Texture::CreateFromFile(texturePath));
            }
        }

        material->SetMaterialName(FastName(materialName));

        FilePath materialPath = currentAssetsFolder + materialName + ".mat";

        Asset<Material> materialAsset = GetEngineContext()->assetManager->CreateAsset<Material>(MaterialAssetLoader::PathKey(materialPath));
        materialAsset->SetMaterial(material);
        GetEngineContext()->assetManager->SaveAsset(materialAsset);

        found = materialCache.emplace(std::make_pair(fbxMaterial, maxVertexInfluence), materialAsset).first;
    }

    return FilePath(GetEngineContext()->assetManager->GetAssetFileInfo(found->second).fileName);
}

void ClearMaterialCache()
{
    FBXMaterialImportDetails::materialCache.clear();
    FBXMaterialImportDetails::currentAssetsFolder = FilePath();
}

}; //ns Details
}; //ns DAVA