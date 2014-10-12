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

BaseProperty *ControlPropertiesSection::CopyAndApplyToOtherControl(DAVA::UIControl *control)
{
    ControlPropertiesSection *section = new ControlPropertiesSection(name);
    
    for (int i = 0; i < GetCount(); i++)
    {
        BaseProperty *prop = GetProperty(i)->CopyAndApplyToOtherControl(control);
        section->AddProperty(prop);
    }
    
    return section;
}
