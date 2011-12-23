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
