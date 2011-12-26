/*
 *  MaterialEditor.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "PropertyList.h"
#include "ComboBox.h"

using namespace DAVA;

class ComboBox;
class MaterialEditor : public DraggableDialog, public UIListDelegate, public ComboBoxDelegate, public PropertyListDelegate
{
public:
    
    MaterialEditor();
    ~MaterialEditor();
    
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    
protected:
    ComboBox *materialsTypes;
    PropertyList *materialProps;
};

#endif