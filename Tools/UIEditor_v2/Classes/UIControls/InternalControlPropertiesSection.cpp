//
//  InternalControlPropertiesSection.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "InternalControlPropertiesSection.h"

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *control, int num) : control(NULL), internalControlNum(num)
{
    this->control = SafeRetain(control);
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(control);
}

DAVA::String InternalControlPropertiesSection::GetName() const
{
    return control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
}
