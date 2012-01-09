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
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue){};
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey){};
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4){};
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
    
    bool IsPropertyAvaliable(const String &propertyName);

//    void AddPropertyByData(PropertyCellData *newProp);

    void AddStringProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddIntProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddFloatProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddFilepathProperty(const String &propertyName, const String &extensionFilter = ".*", editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddBoolProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddComboProperty(const String &propertyName, const Vector<String> &strings, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddMatrix4Property(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddSection(const String &sectionName);

    void SetStringPropertyValue(const String &propertyName, const String &newText);
    void SetIntPropertyValue(const String &propertyName, int32 newIntValue);
    void SetFloatPropertyValue(const String &propertyName, float32 newFloatValue);
    void SetFilepathPropertyValue(const String &propertyName, const String &currentFilepath);
    void SetBoolPropertyValue(const String &propertyName, bool newBoolValue);
    void SetComboPropertyStrings(const String &propertyName, const Vector<String> &strings);
    void SetComboPropertyIndex(const String &propertyName, int32 currentStringIndex);
    void SetMatrix4PropertyValue(const String &propertyName, const Matrix4 &currentMatrix);
    void SetSectionIsOpened(const String &sectionName, bool isOpened);

    
    const String &GetStringPropertyValue(const String &propertyName);
    int32 GetIntPropertyValue(const String &propertyName);
    float32 GetFloatPropertyValue(const String &propertyName);
    const String &GetFilepathPropertyValue(const String &propertyName);
    bool GetBoolPropertyValue(const String &propertyName);
    const String &GetComboPropertyValue(const String &propertyName);
    const int32 GetComboPropertyIndex(const String &propertyName);
    const Matrix4 & GetMatrix4PropertyValue(const String &propertyName);
    bool GetSectionIsOpened(const String &sectrionName);

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    virtual void OnPropertyChanged(PropertyCellData *changedProperty);

    void ReleaseProperties();
    
protected:
    
//    PropertyCellData *GetDataForIndex(int32 index);
    
    void AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType);
    PropertyCellData *PropertyByName(const String &propertyName);
    int32 GetRealIndex(int32 index);
    
    PropertyListDelegate *delegate;
    UIList *propsList;
    Vector<PropertyCellData*> props;
    Map<String, PropertyCellData*> propsMap;
    PropertyCellData *currentSection;
};

#endif