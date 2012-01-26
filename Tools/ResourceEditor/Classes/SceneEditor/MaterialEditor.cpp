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
    
    text = new UIStaticText(Rect(size.x * materialListPart, size.y * previewHeightPart, size.x * materialListPart, 20));
    text->SetFont(ControlsFactory::GetFontLight());
    text->SetText(LocalizedString(L"materialeditor.materialtype"));
    text->SetAlign(ALIGN_RIGHT|ALIGN_VCENTER);
    AddControl(text);
    SafeRelease(text);
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
			materialProps[i]->AddFloatProperty("Diffuse color R");
			materialProps[i]->AddFloatProperty("Diffuse color G");
			materialProps[i]->AddFloatProperty("Diffuse color B");
			materialProps[i]->AddFloatProperty("Diffuse color A");
        }

        if (i == Material::MATERIAL_VERTEX_LIT_TEXTURE
            || i == Material::MATERIAL_VERTEX_LIT_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL
            || i == Material::MATERIAL_NORMAL_MAPPED_SPECULAR)
        {
            materialProps[i]->AddFloatProperty("Specular color R");
            materialProps[i]->AddFloatProperty("Specular color G");
            materialProps[i]->AddFloatProperty("Specular color B");
            materialProps[i]->AddFloatProperty("Specular color A");
        }
        
    }
}

MaterialEditor::~MaterialEditor()
{
    SafeRelease(btnSelected);
    SafeRelease(btnAll);
    
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    SafeRelease(materialsList);
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
    
    SelectMaterial(0);
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
    
    if (workingScene->GetMaterialCount() > 0)
    {
        selectedMaterial = 0;
        SelectMaterial(0);
    }
    
    RefreshList();
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
}

void MaterialEditor::OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex)
{
    if (forComboBox == materialTypes) 
    {
//        Material *mat = workingScene->GetMaterial(selectedMaterial);
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetType((Material::eType)itemIndex);
        PreparePropertiesForMaterialType(mat->type);
    }
}


void MaterialEditor::OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    if (forKey == "Name") 
    {
//        workingScene->GetMaterial(selectedMaterial)->SetName(newValue);
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetName(newValue);
        materialsList->Refresh();
    }
}

void MaterialEditor::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
//    Material *mat = workingScene->GetMaterial(selectedMaterial);
    Material *mat = GetMaterial(selectedMaterial);

    if (forList->IsPropertyAvaliable("Diffuse color R"))
    {
        mat->diffuse.x = forList->GetFloatPropertyValue("Diffuse color R");
        mat->diffuse.y = forList->GetFloatPropertyValue("Diffuse color G");
        mat->diffuse.z = forList->GetFloatPropertyValue("Diffuse color B");
        mat->diffuse.w = forList->GetFloatPropertyValue("Diffuse color A");
    }

    if (forList->IsPropertyAvaliable("Specular color R"))
    {
        mat->specular.x = forList->GetFloatPropertyValue("Specular color R");
        mat->specular.y = forList->GetFloatPropertyValue("Specular color G");
        mat->specular.z = forList->GetFloatPropertyValue("Specular color B");
        mat->specular.w = forList->GetFloatPropertyValue("Specular color A");
    }
    
}

void MaterialEditor::OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue)
{
}

void MaterialEditor::OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue)
{
    if(forKey == "Is Opaque")
    {
//        Material *mat = workingScene->GetMaterial(selectedMaterial);
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetOpaque(newValue);
    }
    else if("materialeditor.twosided" == forKey)
    {
//        Material *mat = workingScene->GetMaterial(selectedMaterial);
        Material *mat = GetMaterial(selectedMaterial);
        mat->SetTwoSided(newValue);
    }
}

void MaterialEditor::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    for (int i = 0; i < ME_TEX_COUNT; i++) 
    {
        if (forKey == textureNames[i]) 
        {
//            Material *mat = workingScene->GetMaterial(selectedMaterial);
            Material *mat = GetMaterial(selectedMaterial);
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

            break;
        }
    }
}


void MaterialEditor::SelectMaterial(int materialIndex)
{
//    Material *mat = workingScene->GetMaterial(materialIndex);
    Material *mat = GetMaterial(materialIndex);
    if(mat)
    {
        PreparePropertiesForMaterialType(mat->type);
        materialTypes->SetSelectedIndex(mat->type, false);
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
    
//    Material *mat = workingScene->GetMaterial(selectedMaterial);
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
                    currentList->SetFilepathPropertyValue(textureNames[i], mat->textures[textureTypes[i]]->relativePathname);
                }
                else 
                {
                    currentList->SetFilepathPropertyValue(textureNames[i], "");
                }
            }
        }
        
        
        currentList->SetBoolPropertyValue("Is Opaque", mat->GetOpaque());
        currentList->SetBoolPropertyValue("materialeditor.twosided", mat->GetTwoSided());
        
        
        
        if (currentList->IsPropertyAvaliable("Diffuse color R"))
        {
            currentList->SetFloatPropertyValue("Diffuse color R", mat->diffuse.x);
            currentList->SetFloatPropertyValue("Diffuse color G", mat->diffuse.y);
            currentList->SetFloatPropertyValue("Diffuse color B", mat->diffuse.z);
            currentList->SetFloatPropertyValue("Diffuse color A", mat->diffuse.w);
        }
        
        if (currentList->IsPropertyAvaliable("Specular color R"))
        {
            currentList->SetFloatPropertyValue("Specular color R", mat->specular.x);
            currentList->SetFloatPropertyValue("Specular color G", mat->specular.y);
            currentList->SetFloatPropertyValue("Specular color B", mat->specular.z);
            currentList->SetFloatPropertyValue("Specular color A", mat->specular.w);
        }
    }    
}


int32 MaterialEditor::ElementsCount(UIList *forList)
{
    if(EDM_ALL == displayMode)
    {
        if (workingScene) 
        {
            return workingScene->GetMaterialCount();
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
    
//    Material *mat = NULL;
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
    selectedMaterial = 0;
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
    selectedMaterial = 0;
    SelectMaterial(0);

    RefreshList();
}

void MaterialEditor::UdpateButtons(bool showButtons)
{
    displayMode = EDM_ALL;
    if(showButtons)
    {
//        displayMode = EDM_SELECTED;
        if(!btnAll->GetParent()) AddControl(btnAll);
        if(!btnSelected->GetParent()) AddControl(btnSelected);
        
        btnAll->SetSelected(false, false);
        btnSelected->SetSelected(false, false);
    }
    else
    {
//        displayMode = EDM_ALL;
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
//    Material *mat = workingScene->GetMaterial(selectedMaterial);
    Material *mat = NULL;
    if(EDM_ALL == displayMode)
    {
        mat = workingScene->GetMaterial(index);
    }
    else
    {
        if(index < workingNodeMaterials.size())
        {
            mat = workingNodeMaterials[index];
        }
    }
    return mat;
}