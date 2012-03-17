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
#include "SceneValidator.h"

static const float32 materialListPart = 0.33;
static const float32 previewHeightPart = 0.5;

static const String textureNames[] = {
    "Diffuse texture"
    , "Decal texture"
    , "Detail texture"
    , "Normal map"};

static const int32 textureTypes[] = {
    Material::TEXTURE_DIFFUSE
    , Material::TEXTURE_DECAL
    , Material::TEXTURE_DETAIL
    , Material::TEXTURE_NORMALMAP};

MaterialEditor::MaterialEditor()
: DraggableDialog(Rect(GetScreenWidth()/8, GetScreenHeight()/8, GetScreenWidth()/4*3, GetScreenHeight()/4*3))
{//todo: create draggable dealog
    
    ControlsFactory::CustomizeDialog(this);
    displayMode = EDM_ALL;
    
    workingMaterial = NULL;
    workingSceneNode = NULL;
    workingScene = NULL;
    float32 materialListWidth = size.x * materialListPart;
    
    btnAll = ControlsFactory::CreateButton(Rect(0, ControlsFactory::BUTTON_HEIGHT, materialListWidth/2, ControlsFactory::BUTTON_HEIGHT), 
                                           LocalizedString(L"materialeditor.all"));
    btnAll->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnAllPressed));
    
    btnSelected = ControlsFactory::CreateButton(Rect(materialListWidth/2, ControlsFactory::BUTTON_HEIGHT, 
                                                     materialListWidth/2, ControlsFactory::BUTTON_HEIGHT), 
                                                LocalizedString(L"materialeditor.selected"));
    btnSelected->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnSelectedPressed));

    
    materialsList = new UIList(Rect(0, ControlsFactory::BUTTON_HEIGHT * 2, 
                                    materialListWidth, size.y - ControlsFactory::BUTTON_HEIGHT * 2), 
                               UIList::ORIENTATION_VERTICAL);
    materialsList->SetDelegate(this);
    ControlsFactory::CusomizeListControl(materialsList);
    AddControl(materialsList);
    UIStaticText *text = new UIStaticText(Rect(0, 0, size.x * materialListPart, ControlsFactory::BUTTON_HEIGHT));
    text->SetFont(ControlsFactory::GetFontLight());
    text->SetText(LocalizedString(L"materialeditor.materials"));
    AddControl(text);
    SafeRelease(text);
    
    int32 textY = (GetRect().dy - ControlsFactory::BUTTON_HEIGHT ) / 2;
    noMaterials = new UIStaticText(Rect(materialListWidth, textY, GetRect().dx - materialListWidth, ControlsFactory::BUTTON_HEIGHT));
    noMaterials->SetFont(ControlsFactory::GetFontLight());
    noMaterials->SetText(LocalizedString(L"materialeditor.nomaterials"));
    
    
    selectedMaterial = -1;
    lastSelection = NULL;
    Vector<String> v;
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        v.push_back(Material::GetTypeName((Material::eType)i));
    }
    
    
    comboboxName = new UIStaticText(Rect(size.x * materialListPart, size.y * previewHeightPart, size.x * materialListPart, 20));
    comboboxName->SetFont(ControlsFactory::GetFontLight());
    comboboxName->SetText(LocalizedString(L"materialeditor.materialtype"));
    comboboxName->SetAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    AddControl(comboboxName);

    materialTypes = new ComboBox(Rect(size.x - size.x * materialListPart, size.y * previewHeightPart, size.x * materialListPart, 20), this, v);
    AddControl(materialTypes);
    
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        materialProps[i] = new PropertyList(Rect(size.x * materialListPart, size.y * previewHeightPart + 25, size.x - size.x * materialListPart, size.y - size.y * previewHeightPart - 25), this);
        materialProps[i]->AddStringProperty("Name");

        materialProps[i]->AddFilepathProperty(textureNames[ME_DIFFUSE], ".png;.pvr");
        if (i == Material::MATERIAL_UNLIT_TEXTURE_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DECAL)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_DECAL], ".png;.pvr");
        }

        if (i == Material::MATERIAL_UNLIT_TEXTURE_DETAIL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_DETAIL], ".png;.pvr");
        }

        if (i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_NORMAL_MAP], ".png;.pvr");
        }
        
        materialProps[i]->AddBoolProperty("Is Opaque");
        materialProps[i]->AddBoolProperty("materialeditor.twosided");
        
        if (i == Material::MATERIAL_VERTEX_LIT_TEXTURE
            || i == Material::MATERIAL_VERTEX_LIT_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP)
        {
            materialProps[i]->AddColorProperty("materialeditor.ambientcolor");
            materialProps[i]->AddColorProperty("materialeditor.diffusecolor");
        }

        if (i == Material::MATERIAL_VERTEX_LIT_TEXTURE
            || i == Material::MATERIAL_VERTEX_LIT_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR
            || i == Material::MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP)
        {
            materialProps[i]->AddColorProperty("materialeditor.specularcolor");
            materialProps[i]->AddFloatProperty("materialeditor.specularshininess");
        }
    }
}

MaterialEditor::~MaterialEditor()
{
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        SafeRelease(materials[k]);
    }
    materials.clear();
    for (int32 k = 0; k < (int32)workingNodeMaterials.size(); ++k)
    {
        SafeRelease(workingNodeMaterials[k]);
    }
    workingNodeMaterials.clear();

    SafeRelease(workingMaterial);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    
    SafeRelease(noMaterials);
    SafeRelease(btnSelected);
    SafeRelease(btnAll);
    SafeRelease(materialsList);
    SafeRelease(comboboxName);
    SafeRelease(materialTypes);
}

void MaterialEditor::UpdateInternalMaterialsVector()
{
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        SafeRelease(materials[k]);
    }
    materials.clear();
    
    workingScene->GetDataNodes(materials);
    
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        materials[k]->Retain();
    }
}

void MaterialEditor::UpdateNodeMaterialsVector()
{
    for (int32 k = 0; k < (int32)workingNodeMaterials.size(); ++k)
    {
        SafeRelease(workingNodeMaterials[k]);
    }

    workingNodeMaterials.clear();
    if(workingSceneNode)
    {
        workingSceneNode->GetDataNodes(workingNodeMaterials);
    }
    else if(workingMaterial)
    {
        workingNodeMaterials.push_back(workingMaterial);
    }
    
    for (int32 k = 0; k < (int32)workingNodeMaterials.size(); ++k)
    {
        workingNodeMaterials[k]->Retain();
    }
}


void MaterialEditor::WillAppear()
{
    UpdateInternalMaterialsVector();
    UpdateNodeMaterialsVector();
    
    OnAllPressed(NULL, NULL, NULL);
}

void MaterialEditor::WillDisappear()
{
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        SafeRelease(materials[k]);
    }
    materials.clear();
    for (int32 k = 0; k < (int32)workingNodeMaterials.size(); ++k)
    {
        SafeRelease(workingNodeMaterials[k]);
    }
    workingNodeMaterials.clear();
    
    SelectMaterial(-1);
    
    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
}


void MaterialEditor::EditMaterial(Scene *newWorkingScene, Material *newWorkingMaterial)
{
    if ((newWorkingScene == workingScene) && (workingMaterial == newWorkingMaterial))
    {
        return;
    }

    SafeRelease(workingMaterial);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    
    workingScene = SafeRetain(newWorkingScene);
    workingMaterial = SafeRetain(newWorkingMaterial);
    
    UdpateButtons(NULL != workingMaterial);

    SelectMaterial(0);
    RefreshList();
}


void MaterialEditor::SetWorkingScene(Scene *newWorkingScene, SceneNode *newWorkingSceneNode)
{
    if ((newWorkingScene == workingScene) && (workingSceneNode == newWorkingSceneNode))
    {
        return;
    }
    
    SafeRelease(workingMaterial);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);

    workingScene = SafeRetain(newWorkingScene);
    workingSceneNode = SafeRetain(newWorkingSceneNode);

    UdpateButtons(NULL != workingSceneNode);
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
}

void MaterialEditor::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    if (forComboBox == materialTypes) 
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->SetType((Material::eType)itemIndex);
            PreparePropertiesForMaterialType(mat->type);
        }
    }
}


void MaterialEditor::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if (forKey == "Name") 
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->SetName(newValue);
            materialsList->Refresh();
        }
    }
}

void MaterialEditor::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{    
    if ("materialeditor.specularshininess" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetShininess(newValue);
    }
}

void MaterialEditor::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("materialeditor.ambientcolor" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetAmbientColor(newColor);
    }
    else if("materialeditor.diffusecolor" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetDiffuseColor(newColor);
    }
    else if("materialeditor.specularcolor" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetSpecularColor(newColor);
    }
}

void MaterialEditor::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
}

void MaterialEditor::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(forKey == "Is Opaque")
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->SetOpaque(newValue);
        }
    }
    else if("materialeditor.twosided" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->SetTwoSided(newValue);
        }
    }
}

void MaterialEditor::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    for (int i = 0; i < ME_TEX_COUNT; i++) 
    {
        if (forKey == textureNames[i]) 
        {
            Material *mat = GetMaterial(selectedMaterial);
            if(mat)
            {
                mat->SetTexture((Material::eTextureLevel)textureTypes[i], newValue);
                Texture *tx = mat->textures[textureTypes[i]];
                if(tx)
                {
                    SceneValidator::Instance()->ValidateTexture(tx);
                }
                else 
                {
                    materialProps[mat->type]->SetFilepathPropertyValue(textureNames[i], "");
                }
                
//                if (mat->textures[textureTypes[i]])
//                {
//                    SafeRelease(mat->textures[textureTypes[i]]);
//                    mat->names[textureTypes[i]] = "";
//                }
//                Texture *tx = Texture::CreateFromFile(newValue);
//                if (tx) 
//                {
//                    mat->textures[textureTypes[i]] = tx;
//                    mat->names[textureTypes[i]] = newValue;
//                    
//                    SceneValidator::Instance()->ValidateTexture(tx);
//                }
//                else 
//                {
//                    //mat->names[textureTypes[i]] = newValue;
//                    materialProps[mat->type]->SetFilepathPropertyValue(textureNames[i], "");
//                }
            }

            break;
        }
    }
}


void MaterialEditor::SelectMaterial(int materialIndex)
{
    Material *mat = GetMaterial(materialIndex);
    if(mat)
    {
        selectedMaterial = materialIndex;
        
        if(!materialTypes->GetParent())
        {
            AddControl(materialTypes);
            AddControl(comboboxName);
        }
        if(noMaterials->GetParent())
        {
            RemoveControl(noMaterials);
        }

        PreparePropertiesForMaterialType(mat->type);
        materialTypes->SetSelectedIndex(mat->type, false);
    }
    else
    {
        selectedMaterial = -1;
        
        for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
        {
            if (materialProps[i]->GetParent())
            {
                RemoveControl(materialProps[i]);
            }
        }
        
        if(materialTypes->GetParent())
        {
            RemoveControl(materialTypes);
            RemoveControl(comboboxName);
        }
        if(!noMaterials->GetParent())
        {
            AddControl(noMaterials);
        }
    }
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
    
    PropertyList *currentList = materialProps[materialType];
    
    Material *mat = GetMaterial(selectedMaterial);
    if(mat)
    {
        currentList->SetStringPropertyValue("Name", mat->GetName());
        for (int i = 0; i < ME_TEX_COUNT; i++) 
        {
            if (currentList->IsPropertyAvaliable(textureNames[i]))
            {
                if (mat->textures[textureTypes[i]])
                {
                    currentList->SetFilepathPropertyValue(textureNames[i], mat->names[textureTypes[i]]);
                }
                else 
                {
                    currentList->SetFilepathPropertyValue(textureNames[i], "");
                }
            }
        }
        
        
        currentList->SetBoolPropertyValue("Is Opaque", mat->GetOpaque());
        currentList->SetBoolPropertyValue("materialeditor.twosided", mat->GetTwoSided());

        if(currentList->IsPropertyAvaliable("materialeditor.ambientcolor"))
        {
            currentList->SetColorPropertyValue("materialeditor.ambientcolor", 
                                               mat->GetAmbientColor());
        }
        
        if(currentList->IsPropertyAvaliable("materialeditor.diffusecolor"))
        {
            currentList->SetColorPropertyValue("materialeditor.diffusecolor", 
                                               mat->GetDiffuseColor());
        }
        
        if(currentList->IsPropertyAvaliable("materialeditor.specularcolor"))
        {
            currentList->SetColorPropertyValue("materialeditor.specularcolor", 
                                               mat->GetSpecularColor());
        }
        
        if(currentList->IsPropertyAvaliable("materialeditor.specularshininess"))
        {
            currentList->SetFloatPropertyValue("materialeditor.specularshininess", mat->GetShininess());
        }
    }    
}


int32 MaterialEditor::ElementsCount(UIList *forList)
{
    if(EDM_ALL == displayMode)
    {
        if (workingScene) 
        {
            return (int32)materials.size();
        }
    }
    else
    {
        return workingNodeMaterials.size();
    }
    return 0;
}

UIListCell *MaterialEditor::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("Material name cell");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, size.x * materialListPart, 20), "Material name cell");

        float32 boxSize = 16;
        float32 y = (CellHeight(forList, index) - boxSize) / 2;
        float32 x = forList->GetRect().dx - boxSize;
        
        Rect r = Rect(x, y, boxSize, boxSize);
        UIControl *sceneFlagBox = new UIControl(r);
        sceneFlagBox->SetName("flagBox");
        sceneFlagBox->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        sceneFlagBox->SetSprite("~res:/Gfx/UI/marker", 1);
        sceneFlagBox->SetInputEnabled(false);
        c->AddControl(sceneFlagBox);
        SafeRelease(sceneFlagBox);
    }

    Material *mat = GetMaterial(index);
    bool found = false;
    if(EDM_ALL == displayMode)
    {
        for (int32 i = 0; i < workingNodeMaterials.size(); ++i) 
        {
            if(workingNodeMaterials[i] == mat)
            {
                found = true;
                break;
            }
        }
    }
    else
    {
        found = true;
    }
    
    ControlsFactory::CustomizeListCell(c, StringToWString(mat->GetName()));
    UIControl *sceneFlagBox = c->FindByName("flagBox");
    sceneFlagBox->SetVisible(found, false);
    
    if (index == selectedMaterial) 
    {
        c->SetSelected(true, false);
        lastSelection = c;
    }
    else
    {
        c->SetSelected(false, false);
    }
    
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
        selectedCell->SetSelected(true, false);
        lastSelection = selectedCell;
        SelectMaterial(selectedCell->GetIndex());
    }    
}


void MaterialEditor::OnAllPressed(BaseObject * object, void * userData, void * callerData)
{
    displayMode = EDM_ALL;
    btnAll->SetSelected(true, false);
    btnSelected->SetSelected(false, false);

    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::OnSelectedPressed(BaseObject * object, void * userData, void * callerData)
{
    displayMode = EDM_SELECTED;

    btnAll->SetSelected(false, false);
    btnSelected->SetSelected(true, false);

    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::UdpateButtons(bool showButtons)
{
    displayMode = EDM_ALL;
    if(showButtons)
    {
        if(!btnAll->GetParent()) AddControl(btnAll);
        if(!btnSelected->GetParent()) AddControl(btnSelected);
        
        btnAll->SetSelected(false, false);
        btnSelected->SetSelected(false, false);
    }
    else
    {
        if(btnAll->GetParent()) RemoveControl(btnAll);
        if(btnSelected->GetParent()) RemoveControl(btnSelected);

        btnAll->SetSelected(false, false);
        btnSelected->SetSelected(false, false);
    }
}

void MaterialEditor::RefreshList()
{
    materialsList->Refresh();
    materialsList->ResetScrollPosition();
}

Material * MaterialEditor::GetMaterial(int32 index)
{
    Material *mat = NULL;
    if(EDM_ALL == displayMode)
    {
        if((0 <= index) && (index < materials.size()))
        {
            mat = materials[index];
        }
    }
    else
    {
        if((0 <= index) && (index < workingNodeMaterials.size()))
        {
            mat = workingNodeMaterials[index];
        }
    }
    return mat;
}