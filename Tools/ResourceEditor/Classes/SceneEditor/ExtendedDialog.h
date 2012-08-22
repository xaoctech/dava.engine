#ifndef __EXTENDED_DIALOG_H__
#define __EXTENDED_DIALOG_H__

#include "DAVAEngine.h"
#include "DraggableDialog.h"

using namespace DAVA;

class ExtendedDialog : public UIControl
{
public:
    
    ExtendedDialog();
    virtual ~ExtendedDialog();
    
    virtual void Close();
    
    virtual void WillAppear();

protected:

    const Rect GetScreenRect() const;
    
    virtual void UpdateSize();
    
    virtual const Rect GetDialogRect() const;
    DraggableDialog *draggableDialog;
};

#endif //__EXTENDED_DIALOG_H__