/*
 *  PropertyValue.h
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROPERTY_VALUE
#define PROPERTY_VALUE

#include "DAVAEngine.h"

using namespace DAVA;
class PropertyCell;
class PropertyCellData : public BaseObject
{
public:
    enum ValueType 
    {
        PROP_VALUE_STRING = 0,
        PROP_VALUE_INTEGER,
        PROP_VALUE_FLOAT,
        PROP_VALUE_BOOL,
        PROP_VALUE_MATRIX4,
        PROP_VALUE_COMBO_BOX,
        PROP_VALUE_SECTION,
        PROP_VALUE_MESSAGE,
        
        PROP_VALUE_COUNT
    };
    
    PropertyCellData(int _valueType);

    int32 GetValueType();

    
    int32 GetInt();
    float32 GetFloat();
    const String& GetString();
    bool GetBool();
    const String& GetExtensionFilter();
    const Vector<String> & GetStringVector();
    int32 GetItemIndex(); 
    const Matrix4 & GetMatrix4() const;
    int32 GetIsSectionOpened(); 
    int32 GetSectionElementsCount();
    const Message & GetMessage();

    void SetInt(int32 newInt);
    void SetFloat(float32 newFloat);
    void SetString(const String& newString);
    void SetBool(bool newBool);
    void SetExtensionFilter(const String& newString);
    void SetStringVector(const Vector<String> &newStrings);
    void SetItemIndex(int32 newItemIndex);
    void SetMatrix4(const Matrix4 & _matrix);
    void SetIsSectionOpened(bool isSectionOpened);
    void SetSectionElementsCount(int32 sectionElementsCount);
    void SetMessage(const Message &newMessage);
    
    int32 cellType;
    String key;
    bool isEditable;
    int index;
    
    PropertyCell *currentCell;
protected:
    int32 valueType;
    
    int32 intValue;
    float32 floatValue;
    String stringValue;
    bool boolValue;

    String extensionFilter;
    
    Vector<String> stringVector;

    Matrix4 matrix4;
    
    Message messageValue;
};

#endif