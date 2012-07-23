/*
 *  DraggableDialog.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FUCKING_ERROR_DIALOG_H__
#define __FUCKING_ERROR_DIALOG_H__

#include "DAVAEngine.h"

using namespace DAVA;

class FuckingErrorDialog : public UIControl
{
public:
    
    FuckingErrorDialog(const Rect &rect, const String &errorMessage);
    virtual ~FuckingErrorDialog();
        
	virtual void Update(float32 timeElapsed);
};

#endif //__FUCKING_ERROR_DIALOG_H__