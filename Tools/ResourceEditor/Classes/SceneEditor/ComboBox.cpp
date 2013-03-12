/*
 *  ComboBox.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/22/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "ComboBox.h"
#include "ControlsFactory.h"

ComboBox::ComboBox(const Rect &rect, ComboBoxDelegate *comboDelegate, const Vector<String> &listItems)
: UIControl(rect)
, maxVisibleItemsCount(0)
{
    delegate = comboDelegate;

    list = NULL;
    comboButton = ControlsFactory::CreateButton(Rect(0,0,size.x,size.y), L"");
    comboButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ComboBox::OnButton));
    AddControl(comboButton);

    SetNewItemsSet(listItems);
}

ComboBox::~ComboBox()
{
    SafeRelease(list);
}

int32 ComboBox::IndexByKey(const String &key)
{
	Map<String, int32>::const_iterator it;
	it = indecesMap.find(key);
    
    DVASSERT(it != indecesMap.end());
    
    return it->second;
}

void ComboBox::SetMaxVisibleItemsCount(int32 itemsCount)
{
	maxVisibleItemsCount = itemsCount;
}

void ComboBox::SetNewItemsSet(const Vector<String> &listItems)
{
    indecesMap.clear();
    items = listItems;

    Font *font = ControlsFactory::GetFont12();
    listWidth = size.x;
    for (int i = 0; i < (int32)items.size(); i++)
    {
        float itemWidth = (float32)font->GetStringSize(StringToWString(items[i])).dx;
        listWidth = Max(listWidth, itemWidth);
        
        indecesMap[items[i]] = i;
    }
    
    if(listWidth != size.x)
    {
        listWidth += (ControlsFactory::OFFSET * 2);
    }
    
//    int32 sz = Min(8, (int32)items.size());
    int32 sz = (int32)items.size();

	if(maxVisibleItemsCount > 0 && sz > maxVisibleItemsCount)
	{
		sz = maxVisibleItemsCount;
	}

    SafeRelease(list);
    list = new UIList(Rect(size.x - listWidth, size.y, listWidth, size.y * sz), UIList::ORIENTATION_VERTICAL);
    list->SetDelegate(this);
    ControlsFactory::SetScrollbar(list);
    ControlsFactory::CusomizeListControl(list);
    
    selectionIndex = 0;
    ControlsFactory::CustomizeButton(comboButton, StringToWString(items[selectionIndex]));
    SetSelectedIndex(0, false);
}


void ComboBox::SetSelectedIndex(int32 newSelecetedIndex, bool reportToDelegatge)
{
    selectionIndex = newSelecetedIndex;
    comboButton->SetStateText(UIControl::STATE_PRESSED_INSIDE, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_DISABLED, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_NORMAL, StringToWString(items[selectionIndex]));
    comboButton->SetStateText(UIControl::STATE_SELECTED, StringToWString(items[selectionIndex]));

        //todo: call delegate
    if (reportToDelegatge && delegate) 
    {
        delegate->OnItemSelected(this, items[selectionIndex], selectionIndex);
    }
    list->Refresh();
}

void ComboBox::SetSelectedKey(const String &newSelecetedKey)
{
    SetSelectedIndex(IndexByKey(newSelecetedKey), true);
}

int32 ComboBox::GetSelectedIndex()
{
    return selectionIndex;
}

const String &ComboBox::GetSelectedKey()
{
    return items[selectionIndex];
}


void ComboBox::OnButton(BaseObject * object, void * userData, void * callerData)
{
    if (list->GetParent()) 
    {
        Cancel();
    }
    else 
    {
        if(parent)
        {
            parent->BringChildFront(this);
        }

        
        UIScreen *screen = UIScreenManager::Instance()->GetScreen();
        if(screen)
        {
            Rect buttonRect = GetRect(true);
            Rect screenRect = screen->GetRect(true);
            
            float32 toRight = (screenRect.x + screenRect.dx) - (buttonRect.x);
            if(toRight < listWidth)
            {
                list->SetPosition(Vector2(buttonRect.x + buttonRect.dx - listWidth, buttonRect.y + buttonRect.dy));
            }
            else 
            {
                list->SetPosition(Vector2(buttonRect.x, buttonRect.y + buttonRect.dy));
            }

            screen->AddControl(list);
        }
        
        comboButton->SetSelected(true);
        list->Refresh();
    }

}

void ComboBox::Cancel()
{
    if (list->GetParent()) 
    {
        list->GetParent()->RemoveControl(list);
        comboButton->SetSelected(false);
    }
}

void ComboBox::Update(float32 timeElapsed)
{
    UIScreen *scr =  UIScreenManager::Instance()->GetScreen();
    if (list->GetParent() == scr) 
    {
        UIControl *f = UIControlSystem::Instance()->GetFocusedControl();
        bool isFocused = false;
        if (f == comboButton || f == list) 
        {
            isFocused = true;
        }
        else 
        {
            if (f && f->GetParent() && f->GetParent()->GetParent() == list)
            {
                isFocused = true;
            }
        }
        if (!isFocused) 
        {
            Cancel();
        }
    }
}


int32 ComboBox::ElementsCount(UIList *forList)
{
    return items.size();
}

UIListCell *ComboBox::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("Combo cell");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, listWidth, size.y), "Combo cell");
    }
    ControlsFactory::CustomizeButton(c, StringToWString(items[index]));
    if (index == selectionIndex) 
    {
        c->SetSelected(true);
    }
    else 
    {
        c->SetSelected(false);
    }

    return c;
}

int32 ComboBox::CellHeight(UIList *forList, int32 index)
{
    return (int32)size.y;
}

void ComboBox::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    SetSelectedIndex(selectedCell->GetIndex(), true);
    comboButton->SetSelected(false);
	if(list->GetParent())
	{
		list->GetParent()->RemoveControl(list);
	}
}

void ComboBox::WillDisappear()
{
    Cancel();
}
