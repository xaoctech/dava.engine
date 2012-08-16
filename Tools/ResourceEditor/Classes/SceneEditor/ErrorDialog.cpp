#include "ErrorDialog.h"
#include "ControlsFactory.h"

ErrorDialog::ErrorDialog()
    :   ExtendedDialog()
{
    draggableDialog->SetRect(GetDialogRect());
    
    errorList = NULL;
    RecreateListControl();
    
    Rect rect = GetDialogRect();
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH) / 2;
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    closeButton = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.close"));
    closeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ErrorDialog::OnCancel));
    draggableDialog->AddControl(closeButton);
}

ErrorDialog::~ErrorDialog()
{
    SafeRelease(closeButton);
    SafeRelease(errorList);
}

void ErrorDialog::RecreateListControl()
{
    if(errorList)
    {
        draggableDialog->RemoveControl(errorList);
        SafeRelease(errorList);
    }
    
    Rect rect = GetDialogRect();
    errorList = new UIList(Rect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT), UIList::ORIENTATION_VERTICAL);
    ControlsFactory::SetScrollbar(errorList);
    errorList->SetDelegate(this);
    draggableDialog->AddControl(errorList);
}

int32 ErrorDialog::ElementsCount(UIList *)
{
    return errorMessages.size();
}

UIListCell *ErrorDialog::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = list->GetReusableCell("ErrorMessage cell"); 
    if(!c)
    {
        c = new UIListCell(Rect(0, 0, list->GetRect().dx, ControlsFactory::ERROR_MESSAGE_HEIGHT), "ErrorMessage cell");
    }
    
    Font *font = ControlsFactory::GetFontError();
    c->SetStateFont(UIControl::STATE_NORMAL, font);
    
    int32 i = 0;
    for(Set<String>::const_iterator it = errorMessages.begin(); it != errorMessages.end(); ++it, ++i)
    {
        if(i == index)
        {
            c->SetStateText(UIControl::STATE_NORMAL, StringToWString(*it));
            UIStaticText *st = c->GetStateTextControl(UIControl::STATE_NORMAL);
            st->SetFittingOption(TextBlock::FITTING_REDUCE);
            break;
        }
    }
    
    return c;
}

int32 ErrorDialog::CellHeight(UIList *, int32)
{
    return (ControlsFactory::ERROR_MESSAGE_HEIGHT);
}

void ErrorDialog::Show(const Set<String> &newErrorMessages)
{
    if(!GetParent())
    {
        errorMessages = newErrorMessages;

        Rect r = GetDialogRect();
        draggableDialog->SetRect(r);

        Vector2 pos = closeButton->GetPosition();
        pos.y = r.dy - ControlsFactory::BUTTON_HEIGHT;
        closeButton->SetPosition(pos);

        RecreateListControl();
        errorList->Refresh();
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


const Rect ErrorDialog::GetDialogRect() const
{
    const Rect screenRect = GetScreenRect();
    
    Rect baseRect(screenRect.dx/8, screenRect.dy/4, screenRect.dx * 3 / 4, screenRect.dy/2);
    int32 height = (errorMessages.size() + 1) * ControlsFactory::ERROR_MESSAGE_HEIGHT;
    if(height + ControlsFactory::BUTTON_HEIGHT < baseRect.dy)
    {
        baseRect.dy = height + ControlsFactory::BUTTON_HEIGHT;
        baseRect.y = (screenRect.dy - baseRect.dy) / 2;
    }
    
    return baseRect;
}

void ErrorDialog::OnCancel(BaseObject *, void *, void *)
{
    Close();
}
