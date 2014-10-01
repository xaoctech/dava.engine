//
//  ControlPropertiesSection.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 30.9.14.
//
//

#include "ControlPropertiesSection.h"

ControlPropertiesSection::ControlPropertiesSection(const DAVA::String &name) : name(name)
{
    
}

DAVA::String ControlPropertiesSection::GetName() const
{
    return name;
}
