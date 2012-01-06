#include "LandscapePropertyControl.h"


LandscapePropertyControl::LandscapePropertyControl(const Rect & rect, bool showMatrix)
    :   NodePropertyControl(rect, showMatrix)
{
    projectPath = "/";
    
    renderingModes.push_back("TEXTURE");
    renderingModes.push_back("SHADER");
    renderingModes.push_back("BLENDED_SHADER");
}

void LandscapePropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    propertyList->AddFloatProperty("Size", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Height", PropertyList::PROPERTY_IS_EDITABLE); 

    propertyList->AddComboProperty("renderingMode", renderingModes);
    
    propertyList->AddFilepathProperty("HeightMap", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_BUMP", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", ".png", PropertyList::PROPERTY_IS_EDITABLE);
    
    
    propertyList->SetStringPropertyValue("Name", "Landscape");
    propertyList->SetFloatPropertyValue("Size", 1.f);
    propertyList->SetFloatPropertyValue("Height", 1.f); 
    
    propertyList->SetComboPropertyIndex("renderingMode", 1);
    
    propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
}


void LandscapePropertyControl::ReadFrom(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFrom(sceneNode);
    
    LandscapeNode *landscape = (LandscapeNode *)sceneNode;
    
    AABBox3 bbox = landscape->GetBoundingBox();
    AABBox3 transformedBox;
    bbox.GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
    
    Vector3 size = transformedBox.max - transformedBox.min;
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
        propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
    }
    
    Texture *t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE0);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
    }

    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTURE1);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
    }

    t = landscape->GetTexture(LandscapeNode::TEXTURE_BUMP);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
    }

    t = landscape->GetTexture(LandscapeNode::TEXTURE_TEXTUREMASK);
    if(t)
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK",t->GetPathname());
    }
    else
    {
        propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
    }
}

void LandscapePropertyControl::WriteTo(SceneNode *sceneNode)
{
    NodePropertyControl::WriteTo(sceneNode);
    
    LandscapeNode *landscape = (LandscapeNode *)sceneNode;
    
    Vector3 size(
                 propertyList->GetFloatPropertyValue("Size"),
				 propertyList->GetFloatPropertyValue("Size"),
                 propertyList->GetFloatPropertyValue("Height"));
    AABBox3 bbox;
    bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
    bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
    
    String renderingModeName = propertyList->GetComboPropertyValue("renderingMode");
    int32 renderingMode = LandscapeNode::RENDERING_MODE_TEXTURE;
    for(int32 i = 0; i < renderingModes.size(); ++i)
    {
        if(renderingModeName == renderingModes[i])
        {
            renderingMode = i;
            break;
        }
    }
    
    String heightMap = propertyList->GetFilepathPropertyValue("HeightMap");
    String texture0 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE0");
    String texture1 = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL");
    String textureBump = propertyList->GetFilepathPropertyValue("TEXTURE_BUMP");
    String textureUnmask = propertyList->GetFilepathPropertyValue("TEXTURE_TEXTUREMASK");
   
    if(IsValidPath(heightMap))
    {
        landscape->BuildLandscapeFromHeightmapImage((LandscapeNode::eRenderingMode)renderingMode, heightMap, bbox);
    }
    
    Texture::EnableMipmapGeneration();
    if(IsValidPath(texture0))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, texture0);
    }

    if(IsValidPath(texture1))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, texture1);
    }

    if(IsValidPath(textureBump))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_BUMP, textureBump);
    }

    if(IsValidPath(textureUnmask))
    {
        landscape->SetTexture(LandscapeNode::TEXTURE_TEXTUREMASK, textureUnmask);
    }
    Texture::DisableMipmapGeneration();
}

void LandscapePropertyControl::SetProjectPath(const String &path)
{
    projectPath = path;
    if('/' != projectPath[projectPath.length() - 1])
    {
        projectPath += '/';
    }
    
    propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
}

bool LandscapePropertyControl::IsValidPath(const String &path)
{
    size_t pos = path.find(".png");
    return (String::npos != pos);
}


