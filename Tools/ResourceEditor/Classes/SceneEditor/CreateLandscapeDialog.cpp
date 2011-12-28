#include "CreateLandscapeDialog.h"
#include "ControlsFactory.h"


CreateLandscapeDialog::CreateLandscapeDialog(const Rect & rect)
    :   CreateNodeDialog(rect)
{
    SetHeader(L"Create Landscape");
}
    
CreateLandscapeDialog::~CreateLandscapeDialog()
{
}

void CreateLandscapeDialog::InitializeProperties()
{
    propertyList->AddStringProperty("Name", "Landscape", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Size", 100.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFloatProperty("Height", 10.f, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("HeightMap", projectPath, ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", projectPath, ".png", PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", ".png", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_BUMP", ".png", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", ".png", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
}

void CreateLandscapeDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new LandscapeNode(scene);

    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
    
    
	Vector3 size(
		propertyList->GetFloatPropertyValue("Size"),
		propertyList->GetFloatPropertyValue("Size"),
		propertyList->GetFloatPropertyValue("Height"));
	AABBox3 bbox;
	bbox.AddPoint(Vector3(-size.x/2.f, -size.y/2.f, 0.f));
	bbox.AddPoint(Vector3(size.x/2.f, size.y/2.f, size.z));


	((LandscapeNode *)sceneNode)->BuildLandscapeFromHeightmapImage(
		LandscapeNode::RENDERING_MODE_DETAIL_SHADER, "~res:/Landscape/hmp2_1.png", bbox);

	Texture::EnableMipmapGeneration();
    ((LandscapeNode *)sceneNode)->SetTexture(LandscapeNode::TEXTURE_TEXTURE0, "~res:/Landscape/tex3.png");
    ((LandscapeNode *)sceneNode)->SetTexture(LandscapeNode::TEXTURE_DETAIL, "~res:/Landscape/detail_gravel.png");
    Texture::DisableMipmapGeneration();

    
    //    propertyList->GetFilepathProperty("HeightMap");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTURE0");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL");
    //    propertyList->GetFilepathProperty("TEXTURE_BUMP");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTUREMASK");
}

void CreateLandscapeDialog::ClearPropertyValues()
{
    propertyList->SetStringPropertyValue("Name", "Landscape");
    
    propertyList->SetFloatPropertyValue("Size", 100.f);
    propertyList->SetFloatPropertyValue("Height", 10.f); 
    propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
}