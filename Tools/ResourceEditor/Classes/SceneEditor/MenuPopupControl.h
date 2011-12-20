#ifndef __MENU_POPUP_CONTROL_H__
#define __MENU_POPUP_CONTROL_H__

#include "DAVAEngine.h"

using namespace DAVA;

class MenuPopupDelegate
{
public:
	virtual void MenuCanceled() = 0;
	virtual void MenuSelected(int32 menuID, int32 itemID) = 0;
    virtual WideString MenuItemText(int32 menuID, int32 itemID) = 0;
    virtual int32 MenuItemsCount(int32 menuID) = 0;
};

class MenuPopupControl: public UIControl, public UIListDelegate
{
    enum eConst
    {
        CELL_HEIGHT = 20, 
    };
    
public:
    MenuPopupControl(const Rect & rect, float32 width, float32 yOffset);
    virtual ~MenuPopupControl();
    
    virtual void WillAppear();

    void SetDelegate(MenuPopupDelegate *delegate);
    
    //list
    virtual int32 ElementsCount(UIList * list);
	virtual UIListCell *CellAtIndex(UIList *list, int32 index);
	virtual int32 CellHeight(UIList * list, int32 index);
	virtual int32 CellWidth(UIList * list, int32 index);
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);

    void InitControl(int32 _menuItemID, const Rect & rect);
    
protected:

    float32 cellWidth;
    
    MenuPopupDelegate *menuDelegate;
    int32 menuItemID;
    
    UIList *menuButtonsList;
    void OnCancel(BaseObject * object, void * userData, void * callerData);
};



#endif // __MENU_POPUP_CONTROL_H__