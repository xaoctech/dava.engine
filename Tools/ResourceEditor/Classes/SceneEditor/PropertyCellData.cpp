/*
 *  PropertyValue.cpp
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "PropertyCellData.h"

PropertyCellData::PropertyCellData(int _valueType)
{
    cellType = PROP_VALUE_COUNT;
    key = "";
    isEditable = false;
    
    index = 0;
    currentCell = NULL;
    
    intValue = 0;
    floatValue = 0.f;
    stringValue = "";
    boolValue = false;

    valueType = _valueType;
}

int32 PropertyCellData::GetValueType()
{
    return valueType;
}

int32 PropertyCellData::GetInt()
{
    DVASSERT(valueType == PROP_VALUE_INTEGER);
    return intValue;
}

float32 PropertyCellData::GetFloat()
{
    DVASSERT(valueType == PROP_VALUE_FLOAT);
    return floatValue;
}

String PropertyCellData::GetString()
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    return stringValue;
}

bool PropertyCellData::GetBool()
{
    DVASSERT(valueType == PROP_VALUE_BOOL);
    return boolValue;
}

void PropertyCellData::SetInt(int32 newInt)
{
    DVASSERT(valueType == PROP_VALUE_INTEGER);
    intValue = newInt;
}

void PropertyCellData::SetFloat(float32 newFloat)
{
    DVASSERT(valueType == PROP_VALUE_FLOAT);
    floatValue = newFloat;
}

void PropertyCellData::SetString(const String& newString)
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    stringValue = newString;
}

void PropertyCellData::SetBool(bool newBool)
{
    DVASSERT(valueType == PROP_VALUE_BOOL);
    boolValue = newBool;
}
