#pragma once

#include "SpriteResourcesPacker.h"

#include <Beast/LightmapAtlasingData.h>

class LightmapsPacker : public SpriteResourcesPacker
{
public:
    void CreateDescriptors();
    void ParseSpriteDescriptors();
    DAVA::Vector<LightmapAtlasingData>* GetAtlasingData();

private:
    DAVA::Vector2 GetTextureSize(const DAVA::FilePath& filePath);

private:
    DAVA::Vector<LightmapAtlasingData> atlasingData;
};
