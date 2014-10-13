#include "InternalControlPropertiesSection.h"

#include "ValueProperty.h"

using namespace DAVA;

InternalControlPropertiesSection::InternalControlPropertiesSection(DAVA::UIControl *control, int num, const InternalControlPropertiesSection *sourceSection) : control(NULL), internalControl(NULL), internalControlNum(num), isContentHidden(false)
{
    this->control = SafeRetain(control);
    
    internalControl = SafeRetain(control->GetInternalControl(num));
    if (internalControl == NULL && sourceSection != NULL && sourceSection->GetInternalControl() != NULL)
    {
        internalControl = control->CreateInternalControl(num);
        control->SetInternalControl(num, internalControl);
    }
    
    if (internalControl)
    {
        const InspInfo *insp = internalControl->GetTypeInfo();
        for (int j = 0; j < insp->MembersCount(); j++)
        {
            const InspMember *member = insp->Member(j);
            ValueProperty *sourceProp = sourceSection == NULL ? NULL : sourceSection->FindProperty(member);
            if (sourceProp && sourceProp->GetValue() != member->Value(internalControl))
                member->SetValue(internalControl, sourceProp->GetValue());
            
            ValueProperty *prop = new ValueProperty(internalControl, member);
            AddProperty(prop);
            SafeRelease(prop);
        }
    }
}

InternalControlPropertiesSection::~InternalControlPropertiesSection()
{
    SafeRelease(internalControl);
    SafeRelease(control);
}

UIControl *InternalControlPropertiesSection::GetInternalControl() const
{
    return internalControl;
}

void InternalControlPropertiesSection::CreateInternalControl()
{
    if (!internalControl)
    {
        internalControl = control->CreateInternalControl(internalControlNum);
        control->SetInternalControl(internalControlNum, internalControl);
    }
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
