
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


void PropertyCellData::SetBool(bool newBool)
{
    DVASSERT(valueType == PROP_VALUE_BOOL);
    boolValue = newBool;
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

int32 PropertyCellData::GetInt()
{
    DVASSERT(valueType == PROP_VALUE_INTEGER);
    return intValue;
}

void PropertyCellData::SetFloat(float32 newFloat)
{
    DVASSERT(valueType == PROP_VALUE_FLOAT);
    floatValue = newFloat;
}

float32 PropertyCellData::GetFloat()
{
    DVASSERT(valueType == PROP_VALUE_FLOAT);
    return floatValue;
}

void PropertyCellData::SetString(const String& newString)
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    stringValue = newString;
}

const String& PropertyCellData::GetString()
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    return stringValue;
}

void PropertyCellData::SetExtensionFilter(const String& newString)
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    extensionFilter = newString;
}

const String& PropertyCellData::GetExtensionFilter()
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    return extensionFilter;
}

void PropertyCellData::SetStringVector(const Vector<String> &newStrings)
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    stringVector = newStrings;
}

const Vector<String> & PropertyCellData::GetStringVector()
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    return stringVector;
}

void PropertyCellData::SetItemIndex(int32 newItemIndex)
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    intValue = newItemIndex;
}

int32 PropertyCellData::GetItemIndex()
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    return intValue;
}

void PropertyCellData::SetMatrix4(const Matrix4 & _matrix)
{
    DVASSERT(valueType == PROP_VALUE_MATRIX4);

    matrix4 = _matrix;
}

const Matrix4 & PropertyCellData::GetMatrix4() const
{
    DVASSERT(valueType == PROP_VALUE_MATRIX4);

    return matrix4;
}

void PropertyCellData::SetIsSectionOpened(bool isSectionOpened)
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    boolValue = isSectionOpened;
}

int32 PropertyCellData::GetIsSectionOpened()
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    return boolValue;
}

void PropertyCellData::SetSectionElementsCount(int32 sectionElementsCount)
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    intValue = sectionElementsCount;
}

int32 PropertyCellData::GetSectionElementsCount()
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    return intValue;
}

const Message & PropertyCellData::GetMessage()
{
    DVASSERT(valueType == PROP_VALUE_MESSAGE);
    return messageValue;
}

void PropertyCellData::SetMessage(const Message &newMessage)
{
    DVASSERT(valueType == PROP_VALUE_MESSAGE);
    messageValue = newMessage;
}

bool PropertyCellData::GetClearDataEnabled()
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    return boolValue;
}

void PropertyCellData::SetClearDataEnabled(bool enabled)
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    boolValue = enabled;
}


