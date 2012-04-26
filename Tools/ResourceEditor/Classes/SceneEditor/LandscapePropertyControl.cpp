#include "LandscapePropertyControl.h"
#include "EditorSettings.h"
#include "SceneValidator.h"

LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    renderingModes.push_back("Texture");
    renderingModes.push_back("Shader");
    renderingModes.push_back("Blended Shader");
    renderingModes.push_back("Tile mask Shader");
}

LandscapePropertyControl::~LandscapePropertyControl()
{

}

void LandscapePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
	DVASSERT(landscape);

    propertyList->AddSection("property.landscape.landscape", GetHeaderState("property.landscape.landscape", true));
    
    propertyList->AddFloatProperty("property.landscape.size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("property.landscape.height", PropertyList::PROPERTY_IS_EDITABLE); 
    
    propertyList->AddComboProperty("property.landscape.renderingmode", renderingModes);
    propertyList->AddFilepathProperty("property.landscape.heightmap", ".png;.heightmap", false, PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddSubsection("property.landscape.subsection.textures");
    propertyList->AddFilepathProperty("property.landscape.texture.color", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture.tile0", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture.tile1", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture.tile2", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture.tile3", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture.tilemask", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddSubsection("property.landscape.subsection.build_mask");
    propertyList->AddFilepathProperty("property.landscape.lightmap", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.alphamask", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    
    
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox;
        bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
        size = transformedBox.max - transformedBox.min;
    }
    
    propertyList->SetFloatPropertyValue("property.landscape.size", size.x);
    propertyList->SetFloatPropertyValue("property.landscape.height", size.z);
    
    propertyList->SetComboPropertyIndex("property.landscape.renderingmode", landscape->GetRenderingMode());
    
    String heightMap = landscape->GetHeightMapPathname();
    if(heightMap.length())
    {
        propertyList->SetFilepathPropertyValue("property.landscape.heightmap", heightMap);
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.heightmap", "");
    }
    Texture * t = 0;
    
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_COLOR);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.color", landscape->GetTextureName(LandscapeNode::TEXTURE_COLOR));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.color", "");
    }

    t = landscape->GetTexture(LandscapeNode::TEXTURE_TILE0);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile0", landscape->GetTextureName(LandscapeNode::TEXTURE_TILE0));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile0", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TILE1);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile1", landscape->GetTextureName(LandscapeNode::TEXTURE_TILE1));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile1", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TILE2);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile2", landscape->GetTextureName(LandscapeNode::TEXTURE_TILE2));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile2", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TILE3);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile3", landscape->GetTextureName(LandscapeNode::TEXTURE_TILE3));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tile3", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tilemask", landscape->GetTextureName(LandscapeNode::TEXTURE_TILE_MASK));
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture.tilemask", "");
    }
    
    
//    t = landscape->GetTexture(LandscapeNode::TEXTURE_BUMP);
//    if(t)
//    {
//        propertyList->SetFilepathPropertyValue("property.landscape.texturebump", landscape->GetTextureName(LandscapeNode::TEXTURE_BUMP));
//    }
//    else
//    {
//        propertyList->SetFilepathPropertyValue("property.landscape.texturebump", "");
//    }
    
    
    propertyList->SetFilepathPropertyValue("property.landscape.lightmap", "");
    propertyList->SetFilepathPropertyValue("property.landscape.alphamask", "");

	propertyList->AddIntProperty("lightmap.size");
	propertyList->SetIntPropertyValue("lightmap.size", currentSceneNode->GetCustomProperties()->GetInt32("lightmap.size", 1024));
    

    propertyList->AddFloatProperty("property.landscape.texture0.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).x);
    propertyList->AddFloatProperty("property.landscape.texture0.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture0.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE0).y);
    
    propertyList->AddFloatProperty("property.landscape.texture1.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).x);
    propertyList->AddFloatProperty("property.landscape.texture1.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture1.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE1).y);

    propertyList->AddFloatProperty("property.landscape.texture2.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).x);
    propertyList->AddFloatProperty("property.landscape.texture2.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture2.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE2).y);
    
    propertyList->AddFloatProperty("property.landscape.texture3.tilex");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tilex", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).x);
    propertyList->AddFloatProperty("property.landscape.texture3.tiley");
    propertyList->SetFloatPropertyValue("property.landscape.texture3.tiley", landscape->GetTextureTiling(LandscapeNode::TEXTURE_TILE3).y);
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.landscape.size" == forKey || "property.landscape.height" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        
        
        int32 renderingMode = propertyList->GetComboPropertyIndex("property.landscape.renderingmode");
        
        String heightMap = propertyList->GetFilepathPropertyValue("property.landscape.heightmap");
        if(EditorSettings::IsValidPath(heightMap) && heightMap.length())
        {
            landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
        }
    }
    if ("property.landscape.texture0.tilex" == forKey || "property.landscape.texture0.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture0.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture0.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE0, tiling);
    }
    if ("property.landscape.texture1.tilex" == forKey || "property.landscape.texture1.tiley" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture1.tilex"),
                       propertyList->GetFloatPropertyValue("property.landscape.texture1.tiley"));
        landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE1, tiling);
    }
	if ("property.landscape.texture2.tilex" == forKey || "property.landscape.texture2.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture2.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture2.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE2, tiling);
	}
	if ("property.landscape.texture3.tilex" == forKey || "property.landscape.texture3.tiley" == forKey)
	{
		LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
		Vector2 tiling(propertyList->GetFloatPropertyValue("property.landscape.texture3.tilex"),
			propertyList->GetFloatPropertyValue("property.landscape.texture3.tiley"));
		landscape->SetTextureTiling(LandscapeNode::TEXTURE_TILE3, tiling);
	}

    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}


void LandscapePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if("lightmap.size" == forKey)
	{
		currentSceneNode->GetCustomProperties()->SetInt32("lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(EditorSettings::IsValidPath(newValue))
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        if("property.landscape.heightmap" == forKey)
        {
            Vector3 size(
                         propertyList->GetFloatPropertyValue("property.landscape.size"),
                         propertyList->GetFloatPropertyValue("property.landscape.size"),
                         propertyList->GetFloatPropertyValue("property.landscape.height"));
            AABBox3 bbox;
            bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
            bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
            
            int32 renderingMode = propertyList->GetComboPropertyIndex("property.landscape.renderingmode");
            if(newValue.length())
            {
                landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, newValue, bbox);
            }
        }
        else if("property.landscape.texture.tile0" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TILE0, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_TILE0));
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texture.tile1" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TILE1, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_TILE1));
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texture.tile2" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TILE2, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_TILE2));
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texture.tile3" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TILE3, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_TILE3));
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texture.tilemask" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TILE_MASK, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK));
            Texture::DisableMipmapGeneration();
        }        
        else if("property.landscape.texture.color" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_COLOR, newValue);
            SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_COLOR));
            Texture::DisableMipmapGeneration();
        }
        else if(    "property.landscape.lightmap" == forKey 
                ||  "property.landscape.alphamask" == forKey)
        {
            String lightMap = propertyList->GetFilepathPropertyValue("property.landscape.lightmap");
            String alphaMask = propertyList->GetFilepathPropertyValue("property.landscape.alphamask");
            
            CreateMaskTexture(lightMap, alphaMask);
        }
    }

    NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnComboIndexChanged(
                                PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("property.landscape.renderingmode" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.size"),
                     propertyList->GetFloatPropertyValue("property.landscape.height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        
        
        int32 renderingMode = propertyList->GetComboPropertyIndex("property.landscape.renderingmode");
        
        String heightMap = propertyList->GetFilepathPropertyValue("property.landscape.heightmap");
        if(EditorSettings::IsValidPath(heightMap) && heightMap.length())
        {
            landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
        }
    }

    NodesPropertyControl::OnComboIndexChanged(forList, forKey, newItemIndex, newItemKey);
}

void LandscapePropertyControl::CreateMaskTexture(const String &lightmapPath, const String &alphamaskPath)
{
    Image *lightMap = Image::CreateFromFile(lightmapPath);
    Image *alphaMask = Image::CreateFromFile(alphamaskPath);
    
    if(lightMap && alphaMask)
    {
        if(     (lightMap->GetPixelFormat() == FORMAT_RGBA8888)
           &&   (alphaMask->GetPixelFormat() == FORMAT_RGBA8888))
        {
            if(     (lightMap->GetHeight() == alphaMask->GetHeight()) 
               &&   (lightMap->GetWidth() == alphaMask->GetWidth()) )
            {
                uint8 *lightMapData = lightMap->GetData();
                uint8 *alphaMaskData = alphaMask->GetData();
                
                int32 dataSize = lightMap->GetHeight() * lightMap->GetWidth() * 4;
                for(int32 i = 0; i < dataSize; i += 4)
                {
                    lightMapData[i + 3] = alphaMaskData[i];
                }
                
                String extension = FileSystem::Instance()->GetExtension(lightmapPath);
                String path, fileName;
                FileSystem::Instance()->SplitPath(lightmapPath, path, fileName);
                
                String resultPath = path + "EditorMaskTexture" + extension;
                FileSystem::Instance()->DeleteFile(resultPath);
                lightMap->Save(resultPath);
                
                propertyList->SetFilepathPropertyValue("property.landscape.lightmap", "");
                propertyList->SetFilepathPropertyValue("property.landscape.alphamask", "");

                propertyList->SetFilepathPropertyValue("property.landscape.texture.color", resultPath);
                LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentSceneNode);
                if(landscape)
                {
                    Texture::EnableMipmapGeneration();
                    landscape->SetTexture(LandscapeNode::TEXTURE_COLOR, resultPath);
                    SceneValidator::Instance()->ValidateTexture(landscape->GetTexture(LandscapeNode::TEXTURE_COLOR));
                    Texture::DisableMipmapGeneration();
                }
            }
        }
    }
    
    SafeRelease(lightMap);
    SafeRelease(alphaMask);
}


