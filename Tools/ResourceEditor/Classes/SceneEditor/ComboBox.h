/*
 *  ComboBox.h
 *  TemplateProjectMacOS
 *
 *  Created by Alexey Prosin on 12/22/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef COMBO_BOX
#define COMBO_BOX

#include "DAVAEngine.h"

using namespace DAVA;

class ComboBox;
class ComboBoxDelegate
{
public:
    virtual void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex){};
};


class ComboBox : public UIControl, public UIListDelegate
{
public:
    
    ComboBox(const Rect &rect, ComboBoxDelegate *comboDelegate, const Vector<String> &listItems);
    ~ComboBox();
    
    void SetNewItemsSet(const Vector<String> &listItems);
    
    
    void SetSelectedIndex(int32 newSelecetedIndex, bool reportToDelegatge);
    void SetSelectedKey(const String &newSelecetedKey);
    int32 GetSelectedIndex();
    const String &GetSelectedKey();
    
    void Cancel();
    
    void OnButton(BaseObject * object, void * userData, void * callerData);

    void Update(float32 timeElapsed);
    
    
    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    
    virtual void WillDisappear();
    
protected:
    int32 IndexByKey(const String &key);

    ComboBoxDelegate *delegate;
    UIList *list;
    Vector<String> items;
    Map<String, int32> indecesMap;
    UIButton *comboButton;
    
    int32 selectionIndex;
    
};

#endif