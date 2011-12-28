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
        
        PROP_VALUE_COUNT
    };
    
    PropertyCellData(int _valueType);

    int32 GetValueType();

    
    int32 GetInt();
    float32 GetFloat();
    const String& GetString();
    bool GetBool();
    const String& GetExtensionFilter();

    void SetInt(int32 newInt);
    void SetFloat(float32 newFloat);
    void SetString(const String& newString);
    void SetBool(bool newBool);
    void SetExtensionFilter(const String& newString);
    
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
};

#endif