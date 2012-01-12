/*
 *  DraggableDialog.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "DraggableDialog.h"

DraggableDialog::DraggableDialog(const Rect &rect)
:UIControl(rect)
{
}

DraggableDialog::~DraggableDialog()
{
}

void DraggableDialog::DidAppear()
{//make a bool flag to keep original position
    originalPosition = relativePosition;
}

void DraggableDialog::DidDisappear()
{
    relativePosition = originalPosition;
}

void DraggableDialog::Input(UIEvent *currentInput)
{
    if (currentInput->phase == UIEvent::PHASE_BEGAN) 
    {
        basePoint = currentInput->point;
    }
    if (currentInput->phase == UIEvent::PHASE_DRAG)
    {
        relativePosition += (currentInput->point - basePoint);
        basePoint = currentInput->point;
        if (parent) 
        {
            if (relativePosition.x > parent->size.x - 10)
            {
                relativePosition.x = parent->size.x - 10;
            }
            if (relativePosition.y > parent->size.y - 10)
            {
                relativePosition.y = parent->size.y - 10;
            }
            if (relativePosition.x + size.x < 10)
            {
                relativePosition.x = 10 - size.x;
            }
            if (relativePosition.y + size.y < 10)
            {
                relativePosition.y = 10 - size.y;
            }
        }
    }
}
