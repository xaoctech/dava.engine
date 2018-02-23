#include "VirtualProperty.h"

#include "PropertyVisitor.h"
#include "../PackageHierarchy/ControlNode.h"
#include "Base/TemplateHelpers.h"
#include "VirtualPropertiesSection.h"

#include "UI/UIControl.h"

using namespace DAVA;

VirtualProperty::VirtualProperty(VirtualPropertiesSection* vSection_, const String& propertyName, const Type* type_)
    : ValueProperty(propertyName, type_)
    , propertyName(propertyName)
    , vSection(vSection_)
{
    DVASSERT(vSection);
}

VirtualProperty::~VirtualProperty()
{
    vSection = nullptr;
}

void VirtualProperty::Refresh(DAVA::int32 refreshFlags)
{
    ValueProperty::Refresh(refreshFlags);

    if ((refreshFlags & REFRESH_DEFAULT_VALUE) != 0 && GetPrototypeProperty())
        ApplyValue(GetDefaultValue());
}

void VirtualProperty::Accept(PropertyVisitor* visitor)
{
}

uint32 VirtualProperty::GetFlags() const
{
    uint32 result = ValueProperty::GetFlags() | EF_CAN_RESET;
    return result;
}

bool VirtualProperty::IsReadOnly() const
{
    return false;
}

VirtualProperty::ePropertyType VirtualProperty::GetType() const
{
    return TYPE_VARIANT;
}

Any VirtualProperty::GetValue() const
{
    return vSection->GetLocalValue(propertyName);
}

bool VirtualProperty::IsOverriddenLocally() const
{
    return vSection->IsOverriddenLocally(propertyName);
}

void VirtualProperty::ResetValue()
{
    vSection->ResetLocalValue(propertyName);
    //SetOverridden(false);
}

void VirtualProperty::ApplyValue(const DAVA::Any& value)
{
    vSection->SetLocalValue(propertyName, value);
}

bool VirtualProperty::IsVirtualProperty() const
{
    return true;
}