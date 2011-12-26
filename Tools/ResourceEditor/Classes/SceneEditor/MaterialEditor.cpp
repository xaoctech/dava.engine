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
static const float32 materialListPart = 0.25;

MaterialEditor::MaterialEditor()
: DraggableDialog(Rect(GetScreenWidth()/8, GetScreenHeight()/8, GetScreenWidth()/4*3, GetScreenHeight()/4*3))
{//todo: create draggable dealog
    ControlsFactory::CustomizePanelControl(this);
    
    workingScene = NULL;
    
    materialsList = new UIList(Rect(0, 20, size.x * materialListPart, size.y - 20), UIList::ORIENTATION_VERTICAL);
    materialsList->SetDelegate(this);
    ControlsFactory::CusomizeListControl(materialsList);
    AddControl(materialsList);
        //    Vector<String> v;
//    v.push_back("Ordinary material");
//    v.push_back("Most ordinary material");
//    v.push_back("Super ordinary material");
//    v.push_back("Fucking best ordinary material");
//    materialsTypes = new ComboBox(Rect(20, 20, 180, 20), NULL, v);
//    AddControl(materialsTypes);
//    
//    materialProps = new PropertyList(Rect(20, 50, 180, 200), this);
//    AddControl(materialProps);
//    materialProps->AddIntProperty("Stat x", 1, PropertyList::PROPERTY_IS_READ_ONLY);
//    materialProps->AddIntProperty("Stat y", 2, PropertyList::PROPERTY_IS_READ_ONLY);
//    materialProps->AddIntProperty("Stat z", 3, PropertyList::PROPERTY_IS_READ_ONLY);
//
//    materialProps->AddIntProperty("x", 1);
//    materialProps->AddIntProperty("y", 2);
//    materialProps->AddIntProperty("z", 3);
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
    materialsList->Refresh();
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
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
}
