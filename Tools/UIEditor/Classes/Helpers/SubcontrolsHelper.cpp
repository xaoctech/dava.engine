//
//  SubcontrolsHelper.cpp
//  UIEditor
//
//  Created by Yuri Coder on 4/1/13.
//
//

#include "SubcontrolsHelper.h"
namespace DAVA {

bool SubcontrolsHelper::ControlIsSubcontrol(UIControl* uiControl)
{
	if (!uiControl || !uiControl->GetParent())
	{
		return false;
	}
	
	// Firstly try the control's method.
	if (uiControl->IsSubcontrol())
	{
		return true;
	}
	
	// Yuri Coder, 2013/04/01. For UIEditor there is no way to compare subcontrols just by pointers,
	// since multiple Clone() calls may change the pointers' values. So currently lets compare by name.
	// TODO: if the parent control will contain children with the same name as one of the subcontrols,
	// this children will become uneditable. Think about the solution.
	const List<UIControl*>& parentSubcontrols = uiControl->GetParent()->GetSubcontrols();
	for (List<UIControl*>::const_iterator iter = parentSubcontrols.begin();
		 iter != parentSubcontrols.end(); iter ++)
	{
		if ((*iter)->GetName() == uiControl->GetName())
		{
			return true;
		}
	}
	
	return false;
}

};