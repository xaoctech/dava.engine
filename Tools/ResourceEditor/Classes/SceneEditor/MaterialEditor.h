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
    
    void SetWorkingScene(Scene *newWorkingScene);
    
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
    
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue);
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue);
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue);
    
    void SelectMaterial(int materialIndex);
    void PreparePropertiesFroMaterialType(int materialType);
    
protected:
    UIList *materialsList;
    ComboBox *materialTypes;
    PropertyList *materialProps[Material::MATERIAL_TYPES_COUNT];
    Scene *workingScene;
    
    int selectedMaterial;
    UIListCell *lastSelection;
};

#endif