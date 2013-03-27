#ifndef __LEVELPERFORMANCETESTIPHONE__TEXTUREHELPER__
#define __LEVELPERFORMANCETESTIPHONE__TEXTUREHELPER__

#include "DAVAEngine.h"
#include "Render/Texture.h"

using DAVA::String;
using DAVA::Map;
using DAVA::Texture;
using DAVA::Landscape;

class TextureHelper
{
public:
	static DAVA::uint32 GetSceneTextureMemory(DAVA::Scene* scene, const String& scenePath);

private:
	static DAVA::uint32 EnumerateSceneTextures(DAVA::Scene* scene, const String& scenePath);

	static void EnumerateTextures(DAVA::Entity *forNode, Map<String, Texture *> &textures);
	static void CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode);
	static void CollectTexture(Map<String, Texture *> &textures, const String &name, Texture *tex);
};

#endif /* defined(__LEVELPERFORMANCETESTIPHONE__TEXTUREHELPER__) */
