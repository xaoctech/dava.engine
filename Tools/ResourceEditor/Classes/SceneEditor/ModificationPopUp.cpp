/*
 *  ModificationPopUp.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Yury Danilov on 12/01/12.
 *  Copyright 2012 DAVA. All rights reserved.
 *
 */

#include "ModificationPopUp.h"
#include "ControlsFactory.h"

ModificationPopUp::ModificationPopUp()
: DraggableDialog(Rect(100, 100, 150, 150))
{
	ControlsFactory::CustomizeDialog(this);
    
    selection = 0;
	
    UIStaticText *text = new UIStaticText(Rect(0, 0, 80, 20));
    text->SetFont(ControlsFactory::GetFont12());
	text->SetTextColor(ControlsFactory::GetColorLight());
    text->SetText(LocalizedString(L"modificationpanel.modification"));
    AddControl(text);
    SafeRelease(text);
    
	parameters = new PropertyList(Rect(0.0f, 20.0f, 100.0f, 100.0f), this);
	parameters->AddFloatProperty("x");
	parameters->AddFloatProperty("y");
	parameters->AddFloatProperty("z");
	AddControl(parameters);
	
	btnReset = ControlsFactory::CreateButton(Rect(0, 130, 80, 20), L"Reset");
	btnReset->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ModificationPopUp::OnReset));
	AddControl(btnReset);
}

ModificationPopUp::~ModificationPopUp()
{
    SafeRelease(parameters);
	SafeRelease(btnReset);
}

void ModificationPopUp::OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue)
{
	if (selection)
	{
		Matrix4 modification;
		modification.Identity();
	
		modification.CreateTranslation(Vector3(forList->GetFloatPropertyValue("x"), 
                                               forList->GetFloatPropertyValue("y"), 
                                               forList->GetFloatPropertyValue("z")));	
		selection->SetLocalTransform(selection->GetLocalTransform() * modification);

		forList->SetFloatPropertyValue("x", 0.0f);
		forList->SetFloatPropertyValue("y", 0.0f);
		forList->SetFloatPropertyValue("z", 0.0f);
	}
}

void ModificationPopUp::OnReset(BaseObject * object, void * userData, void * callerData)
{
	if (selection)
	{
		selection->RestoreOriginalTransforms();
	}
}