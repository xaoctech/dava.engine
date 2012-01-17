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
}

void LandscapePropertyControl::WriteTo(SceneNode * sceneNode)
{
	NodesPropertyControl::WriteTo(sceneNode);

    LandscapeNode *landscape = dynamic_cast<LandscapeNode*> (sceneNode);
	DVASSERT(landscape);

    Vector3 size(
                 propertyList->GetFloatPropertyValue("Size"),
                 propertyList->GetFloatPropertyValue("Size"),
                 propertyList->GetFloatPropertyValue("Height"));
    AABBox3 bbox;
    bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
    bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
    
    
    int32 renderingMode = propertyList->GetComboPropertyIndex("renderingMode");
    
    String heightMap = propertyList->GetFilepathPropertyValue("HeightMap");
    String texture0 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE0");
    String texture1 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL");
    String textureBump = propertyList->GetFilepathPropertyValue("TEXTURE_BUMP");
    String textureUnmask = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTUREMASK");
    
    if(EditorSettings::IsValidPath(heightMap))
    {
        if(heightMap.length())
        {
            landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
        }
    }
    
    Texture::EnableMipmapGeneration();
    if(EditorSettings::IsValidPath(texture0))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, texture0);
    }
    
    if(EditorSettings::IsValidPath(texture1))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, texture1);
    }
    
    if(EditorSettings::IsValidPath(textureBump))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_BUMP, textureBump);
    }
    
    if(EditorSettings::IsValidPath(textureUnmask))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, textureUnmask);
    }
    Texture::DisableMipmapGeneration();
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

