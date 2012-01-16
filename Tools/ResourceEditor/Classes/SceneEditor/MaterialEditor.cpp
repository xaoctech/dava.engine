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
    
//    ControlsFactory::CustomizePanelControl(this);
    ControlsFactory::CustomizeDialog(this);
    
    workingSceneNode = NULL;
    workingScene = NULL;
    workingNodeMaterials = NULL;
    
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

        materialProps[i]->AddFilepathProperty(textureNames[ME_DIFFUSE], ".png");
        if (i == Material::MATERIAL_UNLIT_TEXTURE_DECAL
            || i == Material::MATERIAL_VERTEX_LIT_DECAL)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_DECAL], ".png");
        }

        if (i == Material::MATERIAL_UNLIT_TEXTURE_DETAIL
            || i == Material::MATERIAL_VERTEX_LIT_DETAIL)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_DETAIL], ".png");
        }

        if (i == Material::MATERIAL_NORMAL_MAPPED_DIFFUSE
            || i == Material::MATERIAL_NORMAL_MAPPED_SPECULAR)
        {
            materialProps[i]->AddFilepathProperty(textureNames[ME_NORMAL_MAP], ".png");
        }
        
        materialProps[i]->AddBoolProperty("Is Opaque");
        
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
    SafeRelease(workingNodeMaterials);
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
            for(int32 child = 0; child < workingNodeMaterials->GetChildrenCount(); ++child)
            {
                DataNode *m = workingNodeMaterials->GetChild(child);
                if(m == meshMaterials[iMesh])
                {
                    found = true;
                    break;
                }
            }
            
            if(!found)
            {
                workingNodeMaterials->AddNode(meshMaterials[iMesh]);
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
    SafeRelease(workingNodeMaterials);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    
    workingScene = SafeRetain(newWorkingScene);
    workingNodeMaterials = new DataNode(workingScene);
    workingNodeMaterials->AddNode(material);
    
    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
    }
    lastSelection = NULL;
    if (workingScene->GetMaterialCount() > 0)
    {
        selectedMaterial = 0;
        for (int32 i = 0; i < workingScene->GetMaterialCount(); ++i) 
        {
            Material *mat = workingScene->GetMaterial(i);
            if(mat == material)
            {
                selectedMaterial = i;
            }
        }
        
        SelectMaterial(selectedMaterial);
    }
    materialsList->Refresh();
    materialsList->ResetScrollPosition();
}


void MaterialEditor::SetWorkingScene(Scene *newWorkingScene, SceneNode *selectedSceneNode)
{
    if (newWorkingScene == workingScene && workingSceneNode == selectedSceneNode) 
    {
        return;
    }
    
    SafeRelease(workingNodeMaterials);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);

    workingScene = SafeRetain(newWorkingScene);
    workingSceneNode = SafeRetain(selectedSceneNode);
    workingNodeMaterials = new DataNode(workingScene);
    EnumerateNodeMaterials(NULL);
    
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
    Material *mat = workingScene->GetMaterial(selectedMaterial);
    

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
        Material *mat = workingScene->GetMaterial(selectedMaterial);
        mat->isOpaque = newValue;
    }
}

void MaterialEditor::OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue)
{
    for (int i = 0; i < ME_TEX_COUNT; i++) 
    {
        if (forKey == textureNames[i]) 
        {
            Material *mat = workingScene->GetMaterial(selectedMaterial);
            if (mat->textures[textureTypes[i]])
            {
                SafeRelease(mat->textures[textureTypes[i]]);
            }
            Texture *tx = Texture::CreateFromFile(newValue);
            if (tx) 
            {
                mat->textures[textureTypes[i]] = tx;
            }
            else 
            {
                materialProps[mat->type]->SetFilepathPropertyValue(textureNames[i], "");
            }

            break;
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
    
    PropertyList *currentList = materialProps[materialType];
    
    Material *mat = workingScene->GetMaterial(selectedMaterial);
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
    
    
    currentList->SetBoolPropertyValue("Is Opaque", mat->isOpaque);

    
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
    
    Material *mat = workingScene->GetMaterial(index);
    
    bool found = false;
    for (int32 i = 0; i < workingNodeMaterials->GetChildrenCount(); ++i) 
    {
        if(workingNodeMaterials->GetChild(i) == mat)
        {
            found = true;
            break;
        }
    }
    
    if(found)
    {
        ControlsFactory::CustomizeListCellAlternative(c, StringToWString(mat->GetName()));
    }
    else
    {
        ControlsFactory::CustomizeListCell(c, StringToWString(mat->GetName()));
    }
    
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
