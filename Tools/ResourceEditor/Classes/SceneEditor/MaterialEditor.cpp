/*
 *  MaterialEditor.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "MaterialEditor.h"
#include "ControlsFactory.h"
static const float32 materialListPart = 0.33;
static const float32 previewHeightPart = 0.5;

MaterialEditor::MaterialEditor()
: DraggableDialog(Rect(GetScreenWidth()/8, GetScreenHeight()/8, GetScreenWidth()/4*3, GetScreenHeight()/4*3))
{//todo: create draggable dealog
    ControlsFactory::CustomizePanelControl(this);
    
    workingScene = NULL;
    
    materialsList = new UIList(Rect(0, 20, size.x * materialListPart, size.y - 20), UIList::ORIENTATION_VERTICAL);
    materialsList->SetDelegate(this);
    ControlsFactory::CusomizeListControl(materialsList);
    AddControl(materialsList);
    UIStaticText *text = new UIStaticText(Rect(0, 0, size.x * materialListPart, 20));
    text->SetFont(ControlsFactory::GetFontLight());
    text->SetText(L"Materials :");
    AddControl(text);
    SafeRelease(text);
    
    
    selectedMaterial = -1;
    lastSelection = NULL;
    Vector<String> v;
    v.push_back("UNLIT");
    v.push_back("UNLIT_DETAIL");
    v.push_back("UNLIT_DECAL");
    v.push_back("VERTEX_LIT");
    v.push_back("VERTEX_LIT_DETAIL");
    v.push_back("VERTEX_LIT_DECAL");
    v.push_back("NORMAL_MAPPED_DIFFUSE");
    v.push_back("NORMAL_MAPPED_SPECULAR");
    text = new UIStaticText(Rect(size.x * materialListPart, size.y * previewHeightPart, size.x * materialListPart, 20));
    text->SetFont(ControlsFactory::GetFontLight());
    text->SetText(L"Material type :");
    text->SetAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    AddControl(text);
    SafeRelease(text);
    materialTypes = new ComboBox(Rect(size.x - size.x * materialListPart, size.y * previewHeightPart, size.x * materialListPart, 20), this, v);
    AddControl(materialTypes);
    
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        materialProps[i] = new PropertyList(Rect(size.x * materialListPart, size.y * previewHeightPart + 25, size.x - size.x * materialListPart, size.y - size.y * previewHeightPart - 25), this);
        materialProps[i]->AddStringProperty("Name");
        materialProps[i]->SetStringPropertyValue("Name", " ");

        materialProps[i]->AddFilepathProperty("Diffuse texture", ".png");
        materialProps[i]->SetFilepathPropertyValue("Diffuse texture", " ");
    }
    
}

MaterialEditor::~MaterialEditor()
{
    SafeRelease(workingScene);
    SafeRelease(materialsList);
}

void MaterialEditor::SetWorkingScene(Scene *newWorkingScene)
{
    if (newWorkingScene == workingScene) 
    {
        return;
    }
    SafeRelease(workingScene);
    workingScene = SafeRetain(newWorkingScene);
    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
    }
    lastSelection = NULL;
    if (workingScene->GetMaterialCount() > 0)
    {
        selectedMaterial = 0;
        SelectMaterial(0);
    }
    materialsList->Refresh();
    materialsList->ResetScrollPosition();
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
}

void MaterialEditor::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    if (forComboBox == materialTypes) 
    {
        Material *mat = workingScene->GetMaterial(selectedMaterial);
        mat->SetType((Material::eType)itemIndex);
        PreparePropertiesForMaterialType(mat->type);
    }
}


void MaterialEditor::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if (forKey == "Name") 
    {
        workingScene->GetMaterial(selectedMaterial)->SetName(newValue);
        materialsList->Refresh();
    }
}

void MaterialEditor::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
}

void MaterialEditor::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
}

void MaterialEditor::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
}

void MaterialEditor::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if (forKey == "Diffuse texture") 
    {
        Material *mat = workingScene->GetMaterial(selectedMaterial);
        if (mat->textures[Material::TEXTURE_DIFFUSE])
        {
            SafeRelease(mat->textures[Material::TEXTURE_DIFFUSE]);
        }
        Texture *tx = Texture::CreateFromFile(newValue);
        if (tx) 
        {
            mat->textures[Material::TEXTURE_DIFFUSE] = tx;
        }
        else 
        {
            materialProps[mat->type]->SetFilepathPropertyValue("Diffuse texture", " ");
        }

    }
}


void MaterialEditor::SelectMaterial(int materialIndex)
{
    Material *mat = workingScene->GetMaterial(materialIndex);
    PreparePropertiesForMaterialType(mat->type);
    materialTypes->SetSelectedIndex(mat->type, false);
}

void MaterialEditor::PreparePropertiesForMaterialType(int materialType)
{
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        if (materialProps[i]->GetParent())
        {
            RemoveControl(materialProps[i]);
        }
    }
    AddControl(materialProps[materialType]);
    
    Material *mat = workingScene->GetMaterial(selectedMaterial);
    materialProps[materialType]->SetStringPropertyValue("Name", mat->GetName());
    if (mat->textures[Material::TEXTURE_DIFFUSE])
    {
        materialProps[materialType]->SetFilepathPropertyValue("Diffuse texture", mat->textures[Material::TEXTURE_DIFFUSE]->relativePathname);
    }
    else 
    {
        materialProps[materialType]->SetFilepathPropertyValue("Diffuse texture", " ");
    }


}


int32 MaterialEditor::ElementsCount(UIList *forList)
{
    if (workingScene) 
    {
        return workingScene->GetMaterialCount();
    }
    return 0;
}

UIListCell *MaterialEditor::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("Material name cell");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, size.x * materialListPart, 20), "Material name cell");
    }
    ControlsFactory::CustomizeListCell(c, StringToWString(workingScene->GetMaterial(index)->GetName()));
    if (index == selectedMaterial) 
    {
        c->SetSelected(true, false);
        lastSelection = c;
    }
//    if (index == selectionIndex) 
//    {
//        c->SetSelected(true);
//    }
//    else 
//    {
//        c->SetSelected(false);
//    }
    
    return c;
}

int32 MaterialEditor::CellHeight(UIList *forList, int32 index)
{
    return 20;
}

void MaterialEditor::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if (selectedCell->GetIndex() != selectedMaterial)
    {
        
        if (lastSelection && lastSelection->GetIndex() == selectedMaterial) 
        {
            lastSelection->SetSelected(false, false);
        }
        selectedMaterial = selectedCell->GetIndex();
        selectedCell->SetSelected(true, false);
        lastSelection = selectedCell;
        SelectMaterial(selectedMaterial);

    }
    
}
