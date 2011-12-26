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

MaterialEditor::MaterialEditor()
: DraggableDialog(Rect(GetScreenWidth()/8, GetScreenHeight()/8, GetScreenWidth()/4*3, GetScreenHeight()/4*3))
{//todo: create draggable dealog
    ControlsFactory::CustomizePanelControl(this);
    Vector<String> v;
    v.push_back("Ordinary material");
    v.push_back("Most ordinary material");
    v.push_back("Super ordinary material");
    v.push_back("Fucking best ordinary material");
    materialsTypes = new ComboBox(Rect(20, 20, 180, 20), NULL, v);
    AddControl(materialsTypes);
    
    materialProps = new PropertyList(Rect(20, 50, 180, 200), this);
    AddControl(materialProps);
    materialProps->AddIntProperty("Stat x", 1, PropertyList::PROPERTY_IS_READ_ONLY);
    materialProps->AddIntProperty("Stat y", 2, PropertyList::PROPERTY_IS_READ_ONLY);
    materialProps->AddIntProperty("Stat z", 3, PropertyList::PROPERTY_IS_READ_ONLY);

    materialProps->AddIntProperty("x", 1);
    materialProps->AddIntProperty("y", 2);
    materialProps->AddIntProperty("z", 3);
}

MaterialEditor::~MaterialEditor()
{
}

void MaterialEditor::OnButton(BaseObject * object, void * userData, void * callerData)
{
}

int32 MaterialEditor::ElementsCount(UIList *forList)
{
    return 20;
}

UIListCell *MaterialEditor::CellAtIndex(UIList *forList, int32 index)
{
    return NULL;
}

int32 MaterialEditor::CellHeight(UIList *forList, int32 index)
{
    return 20;
}

void MaterialEditor::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
}
