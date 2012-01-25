#include "ExtendedDialog.h"
#include "ControlsFactory.h"

ExtendedDialog::ExtendedDialog()
    :   UIControl(UIScreenManager::Instance()->GetScreen()->GetRect())
{
    ControlsFactory::CustomizeDialogFreeSpace(this);
    
    draggableDialog = new DraggableDialog(DialogRect());
    ControlsFactory::CustomizeDialog(draggableDialog);
    AddControl(draggableDialog);
}

ExtendedDialog::~ExtendedDialog()
{
    SafeRelease(draggableDialog);
}

const Rect ExtendedDialog::DialogRect()
{
    return Rect(GetRect().dx/4, GetRect().dy/4, GetRect().dx / 2, GetRect().dy/2);
}
