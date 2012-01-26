#include "ErrorDialog.h"
#include "ControlsFactory.h"

ErrorDialog::ErrorDialog()
    :   ExtendedDialog()
{
    draggableDialog->SetRect(DialogRect());
    
    Rect rect = DialogRect();
    
    errorList = new UIList(Rect(0, 0, rect.dx, rect.dy - ControlsFactory::BUTTON_HEIGHT), UIList::ORIENTATION_VERTICAL);
    ControlsFactory::SetScrollbar(errorList);
    errorList->SetDelegate(this);
    draggableDialog->AddControl(errorList);
    
    
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

int32 ErrorDialog::ElementsCount(UIList * list)
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
            break;
        }
    }
    
    return c;
}

int32 ErrorDialog::CellHeight(UIList * list, int32 index)
{
    return ControlsFactory::ERROR_MESSAGE_HEIGHT;
}

void ErrorDialog::Show(const Set<String> &newErrorMessages)
{
    if(!GetParent())
    {
        errorMessages = newErrorMessages;
        errorList->Refresh();
        
        Rect r = DialogRect();
        draggableDialog->SetRect(r);
        
        Vector2 pos = closeButton->GetPosition();
        pos.y = r.dy - ControlsFactory::BUTTON_HEIGHT;
        closeButton->SetPosition(pos);
        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        screen->AddControl(this);
    }
}


const Rect ErrorDialog::DialogRect()
{
    Rect baseRect(GetRect().dx/8, GetRect().dy/4, GetRect().dx * 3 / 4, GetRect().dy/2);
    int32 height = (errorMessages.size() + 1) * ControlsFactory::ERROR_MESSAGE_HEIGHT;
    if(height + ControlsFactory::BUTTON_HEIGHT < baseRect.dy)
    {
        baseRect.dy = height + ControlsFactory::BUTTON_HEIGHT;
        baseRect.y = (GetRect().dy - baseRect.dy) / 2;
    }
    
    return baseRect;
}

void ErrorDialog::OnCancel(BaseObject * owner, void * userData, void * callerData)
{
    if(GetParent())
    {
        GetParent()->RemoveControl(this);
    }
}
