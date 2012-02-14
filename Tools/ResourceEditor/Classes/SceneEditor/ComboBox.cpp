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
{
    delegate = comboDelegate;
    items = listItems;
    for (int i = 0; i < items.size(); i++)
    {
        indecesMap[items[i]] = i;
    }
    int32 sz = Min(8, (int32)items.size());
    list = new UIList(Rect(0, size.y, size.x, size.y * sz), UIList::ORIENTATION_VERTICAL);
    list->SetDelegate(this);
    ControlsFactory::SetScrollbar(list);
    ControlsFactory::CusomizeListControl(list);
    

    selectionIndex = 0;
    comboButton = ControlsFactory::CreateButton(Rect(0,0,size.x,size.y), StringToWString(items[selectionIndex]));
    comboButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &ComboBox::OnButton));
    AddControl(comboButton);
    SetSelectedIndex(0, false);
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


void ComboBox::SetNewItemsSet(const Vector<String> &listItems)
{
    indecesMap.clear();
    items = listItems;
    for (int i = 0; i < items.size(); i++)
    {
        indecesMap[items[i]] = i;
    }
    int32 sz = Min(8, (int32)items.size());
    SafeRelease(list);
    list = new UIList(Rect(0, size.y, size.x, size.y * sz), UIList::ORIENTATION_VERTICAL);
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

        Rect r = GetRect(true);
        list->SetPosition(Vector2(r.x, r.y + r.dy));
        UIScreenManager::Instance()->GetScreen()->AddControl(list);
        
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
        c = new UIListCell(Rect(0, 0, size.x, size.y), "Combo cell");
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
    return size.y;
}

void ComboBox::OnCellSelected(UIList *forList, UIListCell *selectedCell)
{
    SetSelectedIndex(selectedCell->GetIndex(), true);
    comboButton->SetSelected(false);
    list->GetParent()->RemoveControl(list);
}

void ComboBox::WillDisappear()
{
    Cancel();
}
