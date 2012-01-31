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
    v.push_back("UNLIT_TEXTURE");
    v.push_back("UNLIT_TEXTURE_DETAIL");
    v.push_back("UNLIT_TEXTURE_DECAL");
    v.push_back("UNLIT_TEXTURE_LIGHTMAP");
    
    v.push_back("VERTEX_LIT_TEXTURE");
    v.push_back("VERTEX_LIT_TEXTURE_DETAIL");
    v.push_back("VERTEX_LIT_TEXTURE_DECAL");
    
    v.push_back("NORMAL_MAPPED_DIFFUSE");
    v.push_back("NORMAL_MAPPED_SPECULAR");
    
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

        if (i == Material::MATERIAL_NORMAL_MAPPED_DIFFUSE
            || i == Material::MATERIAL_NORMAL_MAPPED_SPECULAR)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_NORMAL_MAP], ".png;.pvr");
        }
        
        materialProps[i]->AddBoolProperty("Is Opaque");
        materialProps[i]->AddBoolProperty("materialeditor.twosided");
        
        if (i == Material::MATERIAL_VERTEX_LIT_TEXTURE
            || i == Material::MATERIAL_VERTEX_LIT_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL
            || i == Material::MATERIAL_NORMAL_MAPPED_DIFFUSE
            || i == Material::MATERIAL_NORMAL_MAPPED_SPECULAR)
        {
            materialProps[i]->AddColorProperty("materialeditor.diffusecolor");
        }

        if (i == Material::MATERIAL_VERTEX_LIT_TEXTURE
            || i == Material::MATERIAL_VERTEX_LIT_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL
            || i == Material::MATERIAL_NORMAL_MAPPED_SPECULAR)
        {
            materialProps[i]->AddColorProperty("materialeditor.specularcolor");
        }
    }
}

MaterialEditor::~MaterialEditor()
{
    SafeRelease(noMaterials);
    
    SafeRelease(btnSelected);
    SafeRelease(btnAll);
    
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
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

void MaterialEditor::WillAppear()
{
    UpdateInternalMaterialsVector();
    
    if (0 < materials.size())
    {
        selectedMaterial = 0;
    }
    else
    {
        selectedMaterial = -1;
    }
    SelectMaterial(selectedMaterial);
}

void MaterialEditor::WillDisappear()
{
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        SafeRelease(materials[k]);
    }
    materials.clear();
    workingNodeMaterials.clear();
    
    selectedMaterial = -1;
    SelectMaterial(selectedMaterial);
}


void MaterialEditor::EnumerateNodeMaterials(DAVA::SceneNode *node)
{
    if(!node)
    {
        node = workingSceneNode;
    }
        
    //add materials to list
    MeshInstanceNode *mesh = dynamic_cast<MeshInstanceNode *>(node);
    if(mesh)
    {
        Vector<Material *> meshMaterials = mesh->GetMaterials();
        for(int32 iMesh = 0; iMesh < meshMaterials.size(); ++iMesh)
        {
            bool found = false;
            for(int32 child = 0; child < workingNodeMaterials.size(); ++child)
            {
                if(workingNodeMaterials[child] == meshMaterials[iMesh])
                {
                    found = true;
                    break;
                }
            }
            
            if(!found)
            {
                workingNodeMaterials.push_back(meshMaterials[iMesh]);
            }
        }
    }
    
    if(node)
    {
        int32 count = node->GetChildrenCount();
        for(int32 i = 0; i < count; ++i)
        {
            EnumerateNodeMaterials(node->GetChild(i));    
        }
        
    }
}

void MaterialEditor::EditMaterial(Scene *newWorkingScene, Material *material)
{
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    
    workingScene = SafeRetain(newWorkingScene);
    workingNodeMaterials.clear();
    workingNodeMaterials.push_back(material);

    UdpateButtons(true);

    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    selectedMaterial = 0;
    SelectMaterial(selectedMaterial);
    
    RefreshList();
}


void MaterialEditor::SetWorkingScene(Scene *newWorkingScene, SceneNode *selectedSceneNode)
{
    if (newWorkingScene == workingScene && workingSceneNode == selectedSceneNode) 
    {
        return;
    }
    
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);

    workingScene = SafeRetain(newWorkingScene);
    workingSceneNode = SafeRetain(selectedSceneNode);
    workingNodeMaterials.clear();
    EnumerateNodeMaterials(NULL);
    
    UdpateButtons(NULL != selectedSceneNode);
    
    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    if (0 < materials.size())
    {
        selectedMaterial = 0;
    }
    else
    {
        selectedMaterial = -1;
    }
    SelectMaterial(selectedMaterial);
    
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
    Material *mat = GetMaterial(selectedMaterial);
    if(!mat) return;
}

void MaterialEditor::OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor)
{
    if("materialeditor.diffusecolor" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->diffuse.x = newColor.r;
            mat->diffuse.y = newColor.g;
            mat->diffuse.z = newColor.b;
            mat->diffuse.w = newColor.a;
        }
    }
    else if("materialeditor.specularcolor" == forKey)
    {
        Material *mat = GetMaterial(selectedMaterial);
        if(mat)
        {
            mat->specular.x = newColor.r;
            mat->specular.y = newColor.g;
            mat->specular.z = newColor.b;
            mat->specular.w = newColor.a;
        }
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
                if (mat->textures[textureTypes[i]])
                {
                    SafeRelease(mat->textures[textureTypes[i]]);
                    mat->names[textureTypes[i]] = "";
                }
                Texture *tx = Texture::CreateFromFile(newValue);
                if (tx) 
                {
                    mat->textures[textureTypes[i]] = tx;
                    mat->names[textureTypes[i]] = newValue;
                    
                    SceneValidator::Instance()->ValidateTexture(tx);
                }
                else 
                {
                    //mat->names[textureTypes[i]] = newValue;
                    materialProps[mat->type]->SetFilepathPropertyValue(textureNames[i], "");
                }
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
                    currentList->SetFilepathPropertyValue(textureNames[i], mat->names[i]);
                }
                else 
                {
                    currentList->SetFilepathPropertyValue(textureNames[i], "");
                }
            }
        }
        
        
        currentList->SetBoolPropertyValue("Is Opaque", mat->GetOpaque());
        currentList->SetBoolPropertyValue("materialeditor.twosided", mat->GetTwoSided());
        
        if(currentList->IsPropertyAvaliable("materialeditor.diffusecolor"))
        {
            currentList->SetColorPropertyValue("materialeditor.diffusecolor", 
                                               Color(mat->diffuse.x, mat->diffuse.y, mat->diffuse.z, mat->diffuse.w));
        }
        
        if(currentList->IsPropertyAvaliable("materialeditor.specularcolor"))
        {
            currentList->SetColorPropertyValue("materialeditor.specularcolor", 
                                               Color(mat->specular.x, mat->specular.y, mat->specular.z, mat->specular.w));
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
    }

    
    UIControl *sceneFlagBox = SafeRetain(c->FindByName("flagBox"));
    if(!sceneFlagBox)
    {
        int32 height = 16;
        int32 width = 16;
        float32 y = (CellHeight(forList, index) - height) / 2;
        float32 x = forList->GetRect().dx - width;
        
        Rect r = Rect(x, y, width, height);
        sceneFlagBox = new UIControl(r);
        sceneFlagBox->SetName("flagBox");
        sceneFlagBox->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        sceneFlagBox->SetSprite("~res:/Gfx/UI/marker", 1);
        sceneFlagBox->SetInputEnabled(false);
        c->AddControl(sceneFlagBox);
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
    sceneFlagBox->SetVisible(found, false);
    
    if (index == selectedMaterial) 
    {
        c->SetSelected(true, false);
        lastSelection = c;
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
        selectedMaterial = selectedCell->GetIndex();
        selectedCell->SetSelected(true, false);
        lastSelection = selectedCell;
        SelectMaterial(selectedMaterial);
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
    
    if(0 < materials.size())
    {
        selectedMaterial = 0;
    }
    else
    {
        selectedMaterial = -1;
    }
    SelectMaterial(selectedMaterial);

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
    
    if(0 < workingNodeMaterials.size())
    {
        selectedMaterial = 0;
    }
    else
    {
        selectedMaterial = -1;
    }
    SelectMaterial(selectedMaterial);

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
    if(0 <= index)
    {
        if(EDM_ALL == displayMode)
        {
            mat = materials[index];
        }
        else
        {
            if(index < workingNodeMaterials.size())
            {
                mat = workingNodeMaterials[index];
            }
        }
    }
    return mat;
}