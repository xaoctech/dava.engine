#include "ControlPropertiesSection.h"

#include "ValueProperty.h"

ControlPropertiesSection::ControlPropertiesSection(DAVA::UIControl *control, const DAVA::String &name) : control(SafeRetain(control)), name(name)
{
    
}

ControlPropertiesSection::~ControlPropertiesSection()
{
    SafeRelease(control);
}

DAVA::String ControlPropertiesSection::GetName() const
{
    return name;
}

ControlPropertiesSection *ControlPropertiesSection::CopyAndApplyToOtherControl(DAVA::UIControl *newControl)
{
    ControlPropertiesSection *section = new ControlPropertiesSection(newControl, name);
    
    for (int i = 0; i < GetCount(); i++)
    {
        ValueProperty *prop = new ValueProperty(newControl, children[i]->GetMember());
        section->AddProperty(prop);
    }
    
    return section;
}
