#ifndef __LIGHTMAPS_PACKER_H__
#define __LIGHTMAPS_PACKER_H__

#include "DAVAEngine.h"
using namespace DAVA;

class LightmapsPacker
{
public:
	struct LightmapAtlasingData
	{
		String meshInstanceName;
		String textureName;
		Vector2 uvOffset;
		Vector2 uvScale;
	};

	LightmapsPacker();
	~LightmapsPacker();

	void SetInputDir(const String & inputDir);
	void SetOutputDir(const String & outputDir);
	void Pack();
	void Compress();
	void ParseSpriteDescriptors();

private:
	String inputDir;
	String outputDir;

	Vector<LightmapAtlasingData> lightmapsData;

	Vector2 GetTextureSize(const String & filePath);
};

#endif //__LIGHTMAPS_PACKER_H__
