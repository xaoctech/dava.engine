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

#include "MaterialPropertyControl.h"

static const float32 materialListPart = 0.33;
static const float32 previewHeightPart = 0.5;

#pragma mark  --MaterialEditor
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


    Rect setupRect(GetRect().dx - ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT);
    UIButton *btnSetupFog = ControlsFactory::CreateButton(setupRect, LocalizedString(L"materialeditor.setupfog"));
    btnSetupFog->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnSetupFog));
    AddControl(btnSetupFog);
    SafeRelease(btnSetupFog);

    UIControl *line = ControlsFactory::CreateLine(Rect(GetRect().dx - ControlsFactory::BUTTON_WIDTH*2, ControlsFactory::BUTTON_HEIGHT * 2, ControlsFactory::BUTTON_WIDTH*2, 1), 
                                                  Color::White());
    AddControl(line);
    SafeRelease(line);
    
    Rect fogRect(setupRect.x - ControlsFactory::BUTTON_WIDTH, setupRect.dy + setupRect.y, ControlsFactory::BUTTON_WIDTH * 2, ControlsFactory::BUTTON_HEIGHT * 5);
    fogControl = new FogControl(fogRect, this);
    
    materialsList = new UIList(Rect(0, ControlsFactory::BUTTON_HEIGHT * 2, 
                                    materialListWidth, size.y - ControlsFactory::BUTTON_HEIGHT * 2), 
                               UIList::ORIENTATION_VERTICAL);
    materialsList->SetDelegate(this);
    ControlsFactory::SetScrollbar(materialsList);
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
    
    materialProps = new MaterialPropertyControl(Rect(size.x * materialListPart, 
                                                      size.y * previewHeightPart, 
                                                      size.x - size.x * materialListPart, 
                                                      size.y - size.y * previewHeightPart),
                                                 false);
    materialProps->SetDelegate(this);
    AddControl(materialProps);
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
    
    SafeRelease(materialProps);
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
    materialProps->SetWorkingScene(workingScene);

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
    materialProps->SetWorkingScene(workingScene);

    workingSceneNode = SafeRetain(newWorkingSceneNode);

    UdpateButtons(NULL != workingSceneNode);
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
}

void MaterialEditor::SelectMaterial(int materialIndex)
{
    Material *mat = GetMaterial(materialIndex);
    if(mat)
    {
        selectedMaterial = materialIndex;

        if(noMaterials->GetParent())
        {
            RemoveControl(noMaterials);
        }

        PreparePropertiesForMaterialType(mat->type);
    }
    else
    {
        selectedMaterial = -1;
        
        if (materialProps->GetParent())
        {
            RemoveControl(materialProps);
        }

        if(!noMaterials->GetParent())
        {
            AddControl(noMaterials);
        }
    }
}

void MaterialEditor::PreparePropertiesForMaterialType(int materialType)
{
    if(!materialProps->GetParent())
    {
        AddControl(materialProps);
    }
    
    Material *mat = GetMaterial(selectedMaterial);
    if(mat)
    {
        materialProps->ReadFrom(mat);
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

void MaterialEditor::OnSetupFog(BaseObject * object, void * userData, void * callerData)
{
    if(fogControl)
    {
        AddControl(fogControl);
    }
}


#pragma mark  --NodesPropertyDelegate
void MaterialEditor::NodesPropertyChanged()
{
    RefreshList();
}

#pragma mark --FogControlDelegate
void MaterialEditor::SetupFog(bool enabled, float32 dencity, const DAVA::Color &newColor)
{
    for(int32 i = 0; i < (int32)materials.size(); ++i)
    {
        materials[i]->SetFog(enabled);
        materials[i]->SetFogDensity(dencity);
        materials[i]->SetFogColor(newColor);
    }
    
    if(workingScene)
    {
        Vector<LandscapeNode *> landscapes;
        workingScene->GetChildNodes(landscapes);

        DVASSERT(1 == landscapes.size());
        
        landscapes[0]->SetFog(enabled);
        landscapes[0]->SetFogDensity(dencity);
        landscapes[0]->SetFogColor(newColor);
    }
}
