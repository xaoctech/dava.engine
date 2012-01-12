/*
 *  ModificationPopUp.h
 *  TemplateProjectMacOS
 *
 *  Created by Yury Danilov on 12/01/12.
 *  Copyright 2012 DAVA. All rights reserved.
 *
 */

#ifndef MODIFICATION_POPUP
#define MODIFICATION_POPUP

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
	inline void SetSelection(SceneNode * _selection)
	{
		selection = _selection;
//		parameters->Refresh();
	}
		
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue);
    void OnReset(BaseObject * object, void * userData, void * callerData);
    
protected:

    PropertyList *parameters;
    SceneNode * selection;
	UIButton * btnReset;

};

#endif