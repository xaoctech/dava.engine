/*
 *  DraggableDialog.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef DRAGGABLE_DIALOG
#define DRAGGABLE_DIALOG

#include "DAVAEngine.h"

using namespace DAVA;

class DraggableDialog : public UIControl
{
public:
    
    DraggableDialog(const Rect &rect);
    ~DraggableDialog();
    
	virtual void DidAppear(); 
	virtual void DidDisappear();

	virtual void Input(UIEvent *currentInput);
    
protected:
    Vector2 originalPosition;
    Vector2 basePoint;
};

#endif