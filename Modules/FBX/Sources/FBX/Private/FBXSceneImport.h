#pragma once

#include "FBXUtils.h"

namespace DAVA
{
class Scene;
class Entity;
class FilePath;

namespace FBXImporterDetails
{
FbxScene* ImportFbxScene(FbxManager* fbxManager, const FilePath& fbxPath);
void ProcessSceneHierarchyRecursive(FbxScene* fbxScene, Scene* scene);
void ProcessHierarchyRecursive(FbxNode* fbxNode, Entity* entity);

bool RemoveRedundantEntities(Entity* entity);
void ProcessMeshLODs(Entity* entity);
};
};