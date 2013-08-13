#include "SceneHelper.h"
#include "SceneEditor/SceneValidator.h"
#include "CubemapEditor/MaterialHelper.h"

using namespace DAVA;

void SceneHelper::EnumerateTextures(Entity *forNode, Map<String, Texture *> &textures)
{
	if(!forNode) return;

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

			NMaterial *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < material->GetTextureCount(); ++t)
				{
                    Texture * texture = material->GetTexture(t);
					CollectTexture(textures, texture);
				}
			}

//			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
//			if(instanceMaterial)
//			{
//				CollectTexture(textures, instanceMaterial->GetLightmapName().GetAbsolutePathname(), instanceMaterial->GetLightmap());
//			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeTextures(textures, land);
		}
	}
}

void SceneHelper::CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectTexture(textures, forNode->GetTexture((Landscape::eTextureLevel)t));
	}
}



void SceneHelper::CollectTexture(Map<String, Texture *> &textures, Texture *tex)
{
    if (!tex)return;
    String name = tex->GetPathname().GetAbsolutePathname();
    
	if(!name.empty() && SceneValidator::Instance()->IsPathCorrectForProject(name))
	{
		textures[name] = tex;
	}
}

void SceneHelper::EnumerateDescriptors(DAVA::Entity *forNode, DAVA::Set<DAVA::FilePath> &descriptors)
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

			NMaterial *material = renderBatch->GetMaterial();
			if(material)
			{
				for(int32 t = 0; t < material->GetTextureCount(); ++t)
				{
					CollectDescriptors(descriptors, material->GetTexture(t)->GetPathname());
				}
			}

//			InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
//			if(instanceMaterial)
//			{
//				CollectDescriptors(descriptors, instanceMaterial->GetLightmapName());
//			}
		}

		Landscape *land = dynamic_cast<Landscape *>(ro);
		if(land)
		{
			CollectLandscapeDescriptors(descriptors, land);
		}
	}
}

void SceneHelper::CollectLandscapeDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, DAVA::Landscape *forNode)
{
	for(int32 t = 0; t < Landscape::TEXTURE_COUNT; t++)
	{
		CollectDescriptors(descriptors, forNode->GetTextureName((Landscape::eTextureLevel)t));
	}
}

void SceneHelper::CollectDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, const DAVA::FilePath &pathname)
{
	if(pathname.GetType() == FilePath::PATH_EMPTY)
		return;

	DVASSERT(pathname.IsEqualToExtension(TextureDescriptor::GetDescriptorExtension()));

	if(!pathname.IsEmpty() && SceneValidator::Instance()->IsPathCorrectForProject(pathname))
	{
		descriptors.insert(pathname);
	}
}

void SceneHelper::EnumerateMaterials(DAVA::Entity *forNode, Vector<Material *> &materials)
{
	if(forNode)
	{
		forNode->GetDataNodes(materials);
		//VI: remove skybox materials so they not to appear in the lists
		MaterialHelper::FilterMaterialsByType(materials, DAVA::Material::MATERIAL_SKYBOX);
	}
}
