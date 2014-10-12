#include "InternalControlPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *control, int num) : control(NULL), internalControlNum(num), isContentHidden(false)
{
    this->control = SafeRetain(control);
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(control);
}

PropertiesSection *InternalControlPropertiesSection::CopyAndApplyForNewControl(UIControl *newControl)
{
    InternalControlPropertiesSection *section = new InternalControlPropertiesSection(newControl, internalControlNum);

    if (control->GetInternalControl(internalControlNum) != NULL && newControl->GetInternalControl(internalControlNum) == NULL)
    {
        UIControl *internal = newControl->CreateInternalControl(internalControlNum);
        newControl->SetInternalControl(internalControlNum, internal);
        SafeRelease(internal);
    }
    
    UIControl *internal = newControl->GetInternalControl(internalControlNum);
    if (internal)
    {
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            const InspMember *member = (*it)->GetMember();
            member->SetValue(internal, (*it)->GetValue());
//            ValueProperty *prop = new ValueProperty(internal, member);
//            section->AddProperty(prop);
//            SafeRelease(prop);
        }
    }
    
    return section;
}

DAVA::String InternalControlPropertiesSection::GetName() const
{
    return control->GetInternalControlName(internalControlNum) + control->GetInternalControlDescriptions();
}

int InternalControlPropertiesSection::GetCount() const
{
    return isContentHidden ? 0 : PropertiesSection::GetCount();
}

void InternalControlPropertiesSection::HideContent()
{
    isContentHidden = true;
}
