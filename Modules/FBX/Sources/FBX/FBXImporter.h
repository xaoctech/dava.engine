#pragma once

namespace DAVA
{
class FilePath;
class Scene;
class FBXImporter
{
public:
    static Scene* ConstructSceneFromFBX(const FilePath& fbxPath);
    static bool ConvertToPrefab(const FilePath& fbxPath, const FilePath& prefabPath);
    static bool ConvertAnimations(const FilePath& fbxPath);
};
};