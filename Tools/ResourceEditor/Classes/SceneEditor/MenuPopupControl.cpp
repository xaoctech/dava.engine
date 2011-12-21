#include "MenuPopupControl.h"
#include "ControlsFactory.h"

MenuPopupControl::MenuPopupControl(const Rect & rect, float32 width, float32 yOffset)
    :   UIControl(rect)
{
    menuItemID = -1;
    menuDelegate = NULL;
    
    cellWidth = width;
    Rect r = rect;
    r.x = 0;
    r.dx = width;
    r.y = yOffset;
    r.dy = GetRect().dy - yOffset;

    menuButtonsList = new UIList(r, UIList::ORIENTATION_VERTICAL);
    menuButtonsList->SetDelegate(this);
    AddControl(menuButtonsList);
    
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
        c = new UIListCell(Rect(0, 0, cellWidth, CELL_HEIGHT), "MenuPopup cell");
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

int32 MenuPopupControl::CellWidth(UIList * list, int32 index)
{
    return cellWidth;
}

void MenuPopupControl::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    int32 index = selectedCell->GetIndex();
    if(menuDelegate)
    {
        menuDelegate->MenuSelected(menuItemID, index);
    }
}

void MenuPopupControl::InitControl(int32 _menuItemID, const Rect & rect)
{
    menuItemID = _menuItemID;

    Vector2 position = menuButtonsList->GetPosition();
    position.x = rect.x;
    menuButtonsList->SetPosition(position);
}

