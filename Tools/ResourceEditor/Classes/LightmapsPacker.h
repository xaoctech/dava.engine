#ifndef __LIGHTMAPS_PACKER_H__
#define __LIGHTMAPS_PACKER_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "LightmapAtlasingData.h"
#include "SpritesPacker.h"

class LightmapsPacker : public SpritesPacker
{
public:
	LightmapsPacker();

	void CreateDescriptors();
	void ParseSpriteDescriptors();
	Vector<LightmapAtlasingData> * GetAtlasingData();

private:
	Vector<LightmapAtlasingData> atlasingData;

	Vector2 GetTextureSize(const FilePath & filePath);
};

#endif //__LIGHTMAPS_PACKER_H__
