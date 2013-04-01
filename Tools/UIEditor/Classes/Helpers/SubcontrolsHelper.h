//
//  SubcontrolsHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 4/1/13.
//
//

#ifndef __SUBCONTROLS_HELPER_H__
#define __SUBCONTROLS_HELPER_H__

#include "UI/UIControl.h"

namespace DAVA
{
class SubcontrolsHelper
{
public:
	// Verify whether the control is subcontrol for its parent.
	static bool ControlIsSubcontrol(UIControl* uiControl);
};
}

#endif /* defined(__SUBCONTROLS_HELPER_H__) */
