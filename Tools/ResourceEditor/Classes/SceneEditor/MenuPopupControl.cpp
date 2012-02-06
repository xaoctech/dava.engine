#include "MenuPopupControl.h"
#include "ControlsFactory.h"

MenuPopupControl::MenuPopupControl(const Rect & rect, float32 yOffset)
    :   UIControl(rect)
{
    menuItemID = -1;
    menuDelegate = NULL;
    
    yListOffset = yOffset;

    menuButtonsList = NULL;
    
    this->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MenuPopupControl::OnCancel));
}
    
MenuPopupControl::~MenuPopupControl()
{
    menuItemID = -1;
    menuDelegate = NULL;
    
    SafeRelease(menuButtonsList);
}


void MenuPopupControl::WillAppear()
{
    menuButtonsList->Refresh();
}


void MenuPopupControl::OnCancel(DAVA::BaseObject *object, void *userData, void *callerData)
{
    if(menuDelegate)
    {
        menuDelegate->MenuCanceled();
    }
}


void MenuPopupControl::SetDelegate(MenuPopupDelegate *delegate)
{
    menuDelegate = delegate;
}

int32 MenuPopupControl::ElementsCount(UIList * list)
{
    if(menuDelegate)
    {
        return menuDelegate->MenuItemsCount(menuItemID);
    }
    
    return 0;
}

UIListCell *MenuPopupControl::CellAtIndex(UIList *list, int32 index)
{
    UIListCell *c = list->GetReusableCell("MenuPopup cell"); //try to get cell from the reusable cells store
    if(!c)
    { //if cell of requested type isn't find in the store create new cell
        c = new UIListCell(Rect(0, 0, list->GetRect().dx, CELL_HEIGHT), "MenuPopup cell");
    }
    
    if(menuDelegate)
    {
        WideString text = menuDelegate->MenuItemText(menuItemID, index);
        ControlsFactory::CustomizeMenuPopupCell(c, text);
    }
    
    return c;//returns cell
}

int32 MenuPopupControl::CellHeight(UIList * list, int32 index)
{
    return CELL_HEIGHT;
}

void MenuPopupControl::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    if(menuDelegate)
    {
        int32 index = selectedCell->GetIndex();
        menuDelegate->MenuSelected(menuItemID, index);
    }
}

void MenuPopupControl::InitControl(int32 _menuItemID, const Rect & rect)
{
    RemoveControl(menuButtonsList);
    SafeRelease(menuButtonsList);

    menuItemID = _menuItemID;

    Rect listRect = rect;
    listRect.x = rect.x;
    listRect.y = yListOffset;
    listRect.dy = GetRect().dy - yListOffset;
 
    menuButtonsList = new UIList(listRect, UIList::ORIENTATION_VERTICAL);
    menuButtonsList->SetDelegate(this);
    AddControl(menuButtonsList);
}

