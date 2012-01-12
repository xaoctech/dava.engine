/*
 *  ModificationPopUp.h
 *  TemplateProjectMacOS
 *
 *  Created by Yury Danilov on 12/01/12.
 *  Copyright 2012 DAVA. All rights reserved.
 *
 */

#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "PropertyList.h"

using namespace DAVA;

class ModificationPopUp : public DraggableDialog, public PropertyListDelegate
{
public:
        
    ModificationPopUp();
    ~ModificationPopUp();
	
    void OnButton(BaseObject * object, void * userData, void * callerData);
	void SetSelection(SceneNode * selection);

    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
        
protected:

    PropertyList *parameters;
    SceneNode * selection;
};

#endif