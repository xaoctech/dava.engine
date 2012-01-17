#include "LandscapePropertyControl.h"
#include "EditorSettings.h"

LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{
    renderingModes.push_back("TEXTURE");
    renderingModes.push_back("SHADER");
    renderingModes.push_back("BLENDED_SHADER");
}

LandscapePropertyControl::~LandscapePropertyControl()
{

}

void LandscapePropertyControl::ReadFrom(SceneNode * sceneNode)
{
	NodesPropertyControl::ReadFrom(sceneNode);

    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
	DVASSERT(landscape);

    propertyList->AddSection("Landscape", GetHeaderState("Landscape", true));
    
    propertyList->AddFloatProperty("Size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Height", PropertyList::PROPERTY_IS_EDITABLE); 
    
    propertyList->AddComboProperty("renderingMode", renderingModes);
    
    propertyList->AddFilepathProperty("HeightMap", ".png;.pvr", false, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_BUMP", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    
    propertyList->AddFilepathProperty("LightMap", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("AlphaMask", ".png;.pvr", true, PropertyList::PROPERTY_IS_EDITABLE);
    
    
    Vector3 size(445.0f, 445.0f, 50.f);
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 emptyBox;
    if((emptyBox.min != bbox.min) && (emptyBox.max != bbox.max))
    {
        AABBox3 transformedBox;
        bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
        size = transformedBox.max - transformedBox.min;
    }
    
    propertyList->SetFloatPropertyValue("Size", size.x);
    propertyList->SetFloatPropertyValue("Height", size.z);
    
    propertyList->SetComboPropertyIndex("renderingMode", landscape->GetRenderingMode());
    
    String heightMap = landscape->GetHeightMapPathname();
    if(heightMap.length())
    {
        propertyList->SetFilepathPropertyValue("HeightMap", heightMap);
    }
    else
    {
        propertyList->SetFilepathPropertyValue("HeightMap", "");
    }
    
    Texture *t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE0);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE1);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_BUMP);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", "");
    }
    
    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTUREMASK);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK",t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", "");
    }
    
    propertyList->SetFilepathPropertyValue("LightMap", "");
    propertyList->SetFilepathPropertyValue("AlphaMask", "");
}


void LandscapePropertyControl::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
    if("Size" == forKey || "Height" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        
        
        int32 renderingMode = propertyList->GetComboPropertyIndex("renderingMode");
        
        String heightMap = propertyList->GetFilepathPropertyValue("HeightMap");
        if(EditorSettings::IsValidPath(heightMap) && heightMap.length())
        {
            landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
        }
    }
    NodesPropertyControl::OnFloatPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if(EditorSettings::IsValidPath(newValue))
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
        if("HeightMap" == forKey)
        {
            Vector3 size(
                         propertyList->GetFloatPropertyValue("Size"),
                         propertyList->GetFloatPropertyValue("Size"),
                         propertyList->GetFloatPropertyValue("Height"));
            AABBox3 bbox;
            bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
            bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
            
            int32 renderingMode = propertyList->GetComboPropertyIndex("renderingMode");
            if(newValue.length())
            {
                landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, newValue, bbox);
            }
        }
        else if("TEXTURE_TEXTURE0" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("TEXTURE_TEXTURE1/TEXTURE_DETAIL" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("TEXTURE_BUMP" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_BUMP, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("TEXTURE_TEXTUREMASK" == forKey)
        {
            Texture::EnableMipmapGeneration();
            landscape->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, newValue);
            Texture::DisableMipmapGeneration();
        }
        else if("LightMap" == forKey || "AlphaMask" == forKey)
        {
            String lightMap = propertyList->GetFilepathPropertyValue("LightMap");
            String alphaMask = propertyList->GetFilepathPropertyValue("AlphaMask");
            
            CreateMaskTexture(lightMap, alphaMask);
        }
    }

    NodesPropertyControl::OnFilepathPropertyChanged(forList, forKey, newValue);
}

void LandscapePropertyControl::OnComboIndexChanged(
                                PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey)
{
    if("renderingMode" == forKey)
    {
        LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (currentNode);
        
        Vector3 size(
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Size"),
                     propertyList->GetFloatPropertyValue("Height"));
        AABBox3 bbox;
        bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
        bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
        
        
        int32 renderingMode = propertyList->GetComboPropertyIndex("renderingMode");
        
        String heightMap = propertyList->GetFilepathPropertyValue("HeightMap");
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
                
                propertyList->SetFilepathPropertyValue("LightMap", "");
                propertyList->SetFilepathPropertyValue("AlphaMask", "");

                propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", resultPath);
            }
        }
    }
    
    SafeRelease(lightMap);
    SafeRelease(alphaMask);
}

