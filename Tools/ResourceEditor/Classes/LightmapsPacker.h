#ifndef __LIGHTMAPS_PACKER_H__
#define __LIGHTMAPS_PACKER_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "LightmapAtlasingData.h"

class LightmapsPacker
{
public:
	LightmapsPacker();
	~LightmapsPacker();

	void SetInputDir(const String & inputDir);
	void SetOutputDir(const String & outputDir);
	void Pack();
	void Compress();
	void ParseSpriteDescriptors();
	Vector<LightmapAtlasingData> * GetAtlasingData();

private:
	String inputDir;
	String outputDir;

	Vector<LightmapAtlasingData> atlasingData;

	Vector2 GetTextureSize(const String & filePath);
};

#endif //__LIGHTMAPS_PACKER_H__
