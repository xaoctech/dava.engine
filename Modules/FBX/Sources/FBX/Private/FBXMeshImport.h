#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class Entity;
class FilePath;

namespace FBXImporterDetails
{
void SetGeometryAssetsFolder(const FilePath& filepath);
void ImportMeshToEntity(FbxNode* fbxNode, Entity* entity);
void ClearMeshCache();
};
};