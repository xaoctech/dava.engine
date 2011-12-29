#include "LandscapePropertyControl.h"


LandscapePropertyControl::LandscapePropertyControl(const Rect & rect)
    :   NodePropertyControl(rect)
{
    
}

void LandscapePropertyControl::InitProperties()
{
    NodePropertyControl::InitProperties();
    
    String projectPath = "/";
    propertyList->AddFloatProperty("Size", 1.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Height", 1.f, PropertyList::PROPERTY_IS_EDITABLE); 
    propertyList->AddFilepathProperty("HeightMap", projectPath);
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->AddFilepathProperty("TEXTURE_BUMP", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
//    propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
}

void LandscapePropertyControl::SetDefaultValues()
{
    propertyList->SetStringPropertyValue("Name", "Landscape");
    propertyList->SetFloatPropertyValue("Size", 1.f);
    propertyList->SetFloatPropertyValue("Height", 1.f); 

    String projectPath = "/";
//    propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
//    propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
//    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
}

void LandscapePropertyControl::ReadFromNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadFromNode(sceneNode);
    
//    LandscapeNode *landscape = (LandscapeNode *)sceneNode;
    
//    Vector3 size = landscape->GetSize();
//    propertyList->SetFloatPropertyValue("Length", size.x);
//    propertyList->SetFloatPropertyValue("Width", size.y); 
//    propertyList->SetFloatPropertyValue("Depth", size.z);
//    propertyList->SetFloatPropertyValue("r", cube->GetColor().r);
//    propertyList->SetFloatPropertyValue("g", cube->GetColor().g);
//    propertyList->SetFloatPropertyValue("b", cube->GetColor().b);
//    propertyList->SetFloatPropertyValue("a", cube->GetColor().a);
}

void LandscapePropertyControl::ReadToNode(SceneNode *sceneNode)
{
    NodePropertyControl::ReadToNode(sceneNode);
    
    LandscapeNode *landscape = (LandscapeNode *)sceneNode;
    
    Vector3 size(
                 propertyList->GetFloatPropertyValue("Size"),
				 propertyList->GetFloatPropertyValue("Size"),
                 propertyList->GetFloatPropertyValue("Height"));
    AABBox3 bbox;
    bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
    bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));
    
    
    landscape->BuildLandscapeFromHeightmapImage(LandscapeNode::RENDERING_MODE_DETAIL_SHADER, "~res:/Landscape/hmp2_1.png", bbox);
    
    Texture::EnableMipmapGeneration();
    landscape->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, "~res:/Landscape/tex3.png");
    landscape->SetTexture(LandscapeNode::TEXTURE_DETAIL, "~res:/Landscape/detail_gravel.png");
    Texture::DisableMipmapGeneration();
}


