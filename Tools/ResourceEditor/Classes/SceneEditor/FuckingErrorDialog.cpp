/*
 *  DraggableDialog.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/23/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "FuckingErrorDialog.h"
#include "ControlsFactory.h"

FuckingErrorDialog::FuckingErrorDialog(const Rect &rect, const String &errorMessage)
:UIControl(rect)
{
    GetBackground()->SetColor(Color::Black());
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    
    UIStaticText *errorControl = new UIStaticText(Rect(0, 0, rect.dx, rect.dy));
    errorControl->SetFont(ControlsFactory::GetFontError());
    errorControl->SetMultiline(true);
    errorControl->SetText(StringToWString(errorMessage));
    
    
    AddControl(errorControl);
    SafeRelease(errorControl);
}

FuckingErrorDialog::~FuckingErrorDialog()
{
}


void FuckingErrorDialog::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);

    bool Z = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_Z);
    bool X = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_X);
    bool C = InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_C);
    
    if(Z && X && C)
    {
        if(GetParent())
        {
            GetParent()->RemoveControl(this);
        }
    }
}