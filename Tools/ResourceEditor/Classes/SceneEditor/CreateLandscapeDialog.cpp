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
    propertyList->AddFilepathProperty("HeightMap", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE0", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_BUMP", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
    propertyList->AddFilepathProperty("TEXTURE_TEXTUREMASK", projectPath, PropertyList::PROPERTY_IS_EDITABLE);
}

void CreateLandscapeDialog::CreateNode()
{
    SafeRelease(sceneNode);
    sceneNode = new LandscapeNode(scene);

    sceneNode->SetName(propertyList->GetStringPropertyValue("Name"));
    //    propertyList->GetFilepathProperty("HeightMap");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTURE0");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTURE1/TEXTURE_DETAIL");
    //    propertyList->GetFilepathProperty("TEXTURE_BUMP");
    //    propertyList->GetFilepathProperty("TEXTURE_TEXTUREMASK");
}

void CreateLandscapeDialog::ClearPropertyValues()
{
    propertyList->SetStringPropertyValue("Name", "Landscape");
    propertyList->SetFilepathPropertyValue("HeightMap", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE0", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTURE1/TEXTURE_DETAIL", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_BUMP", projectPath);
    propertyList->SetFilepathPropertyValue("TEXTURE_TEXTUREMASK", projectPath);
}