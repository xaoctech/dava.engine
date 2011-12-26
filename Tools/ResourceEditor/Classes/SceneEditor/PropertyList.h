/*
 *  PropertyList.h
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROPERTY_LIST
#define PROPERTY_LIST

#include "DAVAEngine.h"
#include "PropertyCellData.h"
#include "PropertyCell.h"

using namespace DAVA;

class PropertyList;
class PropertyListDelegate
{
public:
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue){};
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue){};
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue){};
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue){};
};



class PropertyList : public UIControl, public UIListDelegate, public PropertyCellDelegate
{
public:
    
    enum editableType 
    {
        PROPERTY_IS_EDITABLE = 0,
        PROPERTY_IS_READ_ONLY
    };
    
    PropertyList(const Rect &rect, PropertyListDelegate *propertiesDelegate);
    ~PropertyList();

//    void AddPropertyByData(PropertyCellData *newProp);

    void AddTextProperty(const String &propertyName, const String &currentText, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddIntProperty(const String &propertyName, int32 currentIntValue, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddFloatProperty(const String &propertyName, float32 currentFloatValue, editableType propEditType = PROPERTY_IS_EDITABLE);

    void AddFilepathProperty(const String &propertyName, const String &currentFilepath, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddBoolProperty(const String &propertyName, bool currentBoolValue, editableType propEditType = PROPERTY_IS_EDITABLE);

    void SetTextPropertyValue(const String &propertyName, const String &newText);
    void SetIntPropertyValue(const String &propertyName, int32 newIntValue);
    void SetFloatPropertyValue(const String &propertyName, float32 newFloatValue);
    void SetFilepathPropertyValue(const String &propertyName, const String &currentFilepath);
    void SetBoolPropertyValue(const String &propertyName, bool newBoolValue);

    String GetTextPropertyValue(const String &propertyName);
    int32 GetIntPropertyValue(const String &propertyName);
    float32 GetFloatPropertyValue(const String &propertyName);
    String GetFilepathProperty(const String &propertyName);
    bool GetBoolPropertyValue(const String &propertyName);

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    virtual void OnPropertyChanged(PropertyCellData *changedProperty);

    void ReleaseProperties();
    
protected:
    
    
    void AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType);
    PropertyCellData *PropertyByName(const String &propertyName);
    
    PropertyListDelegate *delegate;
    UIList *propsList;
    Vector<PropertyCellData*> props;
    Map<String, PropertyCellData*> propsMap;
};

#endif