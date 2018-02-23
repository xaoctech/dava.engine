#pragma once

#include "FBXUtils.h"

#include "Asset/Asset.h"
#include "Render/Material/Material.h"

namespace DAVA
{
class FilePath;
class NMaterial;

namespace FBXImporterDetails
{
void SetMaterialAssetsFolder(const FilePath& filepath);
FilePath ImportMaterial(const FbxSurfaceMaterial* fbxMaterial, uint32 maxVertexInfluence);
void ClearMaterialCache();
}
};