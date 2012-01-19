#include "LandscapePropertyControl.h"
#include "EditorSettings.h"

LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    renderingModes.push_back("Texture");
    renderingModes.push_back("Shader");
    renderingModes.push_back("Blended Shader");
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
    
    propertyList->AddFilepathProperty("property.landscape.heightmap", ".png;.pvr", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture0", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texture1", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texturebump", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("property.landscape.texturemask", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    
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
    
    Texture *t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE0);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture0", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture0", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE1);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture1", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texture1", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_BUMP);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texturebump", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texturebump", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTUREMASK);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texturemask",t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("property.landscape.texturemask", "");
    }
    
    propertyList->SetFilepathPropertyValue("property.landscape.lightmap", "");
    propertyList->SetFilepathPropertyValue("property.landscape.alphamask", "");

	propertyList->AddIntProperty("lightmap.size");
	propertyList->SetIntPropertyValue("lightmap.size", currentNode->GetCustomProperties()->GetInt32("lightmap.size", 1024));
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("property.landscape.size" == forKey || "property.landscape.height" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
        
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
    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}


void LandscapePropertyControl::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
	if("lightmap.size" == forKey)
	{
		currentNode->GetCustomProperties()->SetInt32("lightmap.size", newValue);
	}

	NodesPropertyControl::OnIntPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(EditorSettings::IsValidPath(newValue))
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
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
        else if("property.landscape.texture0" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texture1" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texturebump" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_BUMP, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("property.landscape.texturemask" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, newValue);
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
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
        
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
        if(     (lightMap->GetPixelFormat() == Image::FORMAT_RGBA8888)
           &&   (alphaMask->GetPixelFormat() == Image::FORMAT_RGBA8888))
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
                lightMap->Save(resultPath);
                
                propertyList->SetFilepathPropertyValue("property.landscape.lightmap", "");
                propertyList->SetFilepathPropertyValue("property.landscape.alphamask", "");

                propertyList->SetFilepathPropertyValue("property.landscape.texturemask", resultPath);
            }
        }
    }
    
    SafeRelease(lightMap);
    SafeRelease(alphaMask);
}


