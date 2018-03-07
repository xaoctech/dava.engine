#include "VirtualPropertiesSection.h"

#include "VarTableValueProperty.h"
#include "PropertyVisitor.h"
#include "ValueProperty.h"

#include <UI/UIControl.h>
#include <UI/Properties/VarTable.h>

#include <TArc/Utils/ReflectionHelpers.h>

using namespace DAVA;

VirtualPropertiesSection::VirtualPropertiesSection(const DAVA::String& name, UIComponent* properiesComponent, VarTableValueProperty* properiesProperty_)
    : SectionProperty(name)
    , properiesProperty(properiesProperty_)
    , properiesComponent(properiesComponent)
{
    DVASSERT(properiesProperty_);
    RebuildVirtualProperties();
}

VirtualPropertiesSection::~VirtualPropertiesSection()
{
    properiesComponent = nullptr;
    properiesProperty = nullptr;
}

void VirtualPropertiesSection::ResetLocalValue(const String& propertyName_)
{
    properiesProperty->ResetLocalValue(propertyName_);
}

Any VirtualPropertiesSection::GetLocalValue(const String& propertyName_)
{
    return properiesProperty->GetLocalValue(propertyName_);
}

void VirtualPropertiesSection::SetLocalValue(const String& propertyName_, const Any& value)
{
    properiesProperty->SetLocalValue(propertyName_, value);
}

bool VirtualPropertiesSection::IsOverriddenLocally(const String& name)
{
    return properiesProperty->IsOverriddenLocally(name);
}

void VirtualPropertiesSection::RebuildVirtualProperties()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        DVASSERT((*it)->GetParent() == this);
        (*it)->SetParent(nullptr);
        (*it)->Release();
    }
    children.clear();

    Any vt = properiesProperty->GetValue();
    if (vt.CanGet<VarTable>())
    {
        VarTable varTable = vt.Get<VarTable>();
        varTable.ForEachProperty([&](const FastName& name, const Any& value) {
            VirtualProperty* prop = new VirtualProperty(this, name.c_str(), value.GetType());
            AddProperty(prop);
            SafeRelease(prop);
        });
    }
    std::sort(children.begin(), children.end(), [](VirtualProperty* a, VirtualProperty* b) {
        return strcmp(a->GetName().c_str(), b->GetName().c_str()) < 0;
    });
}

void VirtualPropertiesSection::Accept(PropertyVisitor* visitor)
{
}

DAVA::UIComponent* VirtualPropertiesSection::GetComponent()
{
    return properiesComponent;
}

bool VirtualPropertiesSection::IsVirtualProperty() const
{
    return true;
}
