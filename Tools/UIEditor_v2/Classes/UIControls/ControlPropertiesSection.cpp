#include "ControlPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

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

PropertiesSection *ControlPropertiesSection::CopyAndApplyForNewControl(UIControl *newControl)
{
    ControlPropertiesSection *section = new ControlPropertiesSection(newControl, name);
    
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        const InspMember *member = (*it)->GetMember();
        member->SetValue(newControl, (*it)->GetValue());
//        ValueProperty *prop = new ValueProperty(newControl, member);
//        section->AddProperty(prop);
//        SafeRelease(prop);
    }
    
    return section;
}
