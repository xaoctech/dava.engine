#include "TextureHelper.h"

#include "Render/LibPVRHelper.h"
#include "Render/TextureDescriptor.h"

using namespace DAVA;

uint32 TextureHelper::GetSceneTextureMemory(DAVA::Scene *scene, const String& scenePath)
{
	return EnumerateSceneTextures(scene, scenePath);
}

uint32 TextureHelper::EnumerateSceneTextures(DAVA::Scene* scene, const String& scenePath)
{
	Map<String, Texture *> textureMap;
	EnumerateTextures(scene, textureMap);

	String projectPath = scenePath;

	uint32 sceneTextureMemory = 0;
	for(Map<String, Texture *>::const_iterator it = textureMap.begin(); it != textureMap.end(); ++it)
	{
		Texture *t = it->second;
		if(String::npos == t->GetPathname().find(projectPath))
		{   // skip all textures that are not related the scene
			continue;
		}

		if(String::npos != t->GetPathname().find(projectPath))
		{   //We need real info about textures size. In Editor on desktop pvr textures are decompressed to RGBA8888, so they have not real size.
			String imageFileName = TextureDescriptor::GetPathnameForFormat(t->GetPathname(), t->GetSourceFileFormat());
			switch (t->GetSourceFileFormat())
			{
				case DAVA::PVR_FILE:
				{
					sceneTextureMemory += LibPVRHelper::GetDataLength(imageFileName);
					break;
				}

				case DAVA::DXT_FILE:
				{
					sceneTextureMemory += (int32)LibDxtHelper::GetDataSize(imageFileName);
					break;
				}

				default:
					sceneTextureMemory += t->GetDataSize();
					break;
			}
		}
	}

	return sceneTextureMemory;
}

void TextureHelper::EnumerateTextures(DAVA::Entity *forNode, Map<String, Texture *> &textures)
{
	if(!forNode)  return;

	Vector<Entity *> nodes;
	forNode->GetChildNodes(nodes);

	nodes.push_back(forNode);

	for(int32 n = 0; n < (int32)nodes.size(); ++n)
	{
		RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
		if(!rc) continue;

		RenderObject *ro = rc->GetRenderObject();
		if(!ro) continue;

		uint32 count = ro->GetRenderBatchCount();
		for(uint32 b = 0; b < count; ++b)
		{
			RenderBatch *renderBatch = ro->GetRenderBatch(b);

			Material *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
				{
					CollectTexture(textures, material->GetTextureName((DAVA::Material::eTextureLevel)t), material->GetTexture((DAVA::Material::eTextureLevel)t));
				}
			}

			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
			if(instanceMaterial)
			{
				CollectTexture(textures, instanceMaterial->GetLightmapName(), instanceMaterial->GetLightmap());
			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeTextures(textures, land);
		}
	}
}

void TextureHelper::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, forNode->GetTextureName((Landscape::eTextureLevel)t), forNode->GetTexture((Landscape::eTextureLevel)t));
	}
}



void TextureHelper::CollectTexture(Map<String, Texture *> &textures, const String &name, Texture *tex)
{
	if (!name.empty())
	{
		textures[name] = tex;
	}
}