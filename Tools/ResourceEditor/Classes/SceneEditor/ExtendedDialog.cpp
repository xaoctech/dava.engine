#include "ExtendedDialog.h"
#include "ControlsFactory.h"

ExtendedDialog::ExtendedDialog()
    :   UIControl(UIScreenManager::Instance()->GetScreen()->GetRect())
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
    draggableDialog = new DraggableDialog(GetDialogRect());
    ControlsFactory::CustomizeDialog(draggableDialog);
    AddControl(draggableDialog);
}

ExtendedDialog::~ExtendedDialog()
{
    SafeRelease(draggableDialog);
}

const Rect ExtendedDialog::GetScreenRect() const
{
    UIScreen *activeScreen = UIScreenManager::Instance()->GetScreen();
    if(activeScreen)
    {
        return activeScreen->GetRect();
    }
    
    return Rect();
}

const Rect ExtendedDialog::GetDialogRect() const
{
    const Rect screenRect = GetScreenRect();
    
    return Rect(screenRect.dx/4, screenRect.dy/4, screenRect.dx / 2, screenRect.dy/2);
}

void ExtendedDialog::Close()
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}


void ExtendedDialog::WillAppear()
{
    UIControl::WillAppear();
    
    UpdateSize();
}

void ExtendedDialog::UpdateSize()
{
    SetRect(GetScreenRect());
    
    draggableDialog->SetRect(GetDialogRect());
}