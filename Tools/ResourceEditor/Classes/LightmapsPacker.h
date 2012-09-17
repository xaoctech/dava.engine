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

	void Compress();
	void ParseSpriteDescriptors();
	Vector<LightmapAtlasingData> * GetAtlasingData();

private:
	String inputDir;
	String outputDir;

	Vector<LightmapAtlasingData> atlasingData;

	Vector2 GetTextureSize(const String & filePath);

	int32 compressFormat;
};

#endif //__LIGHTMAPS_PACKER_H__
