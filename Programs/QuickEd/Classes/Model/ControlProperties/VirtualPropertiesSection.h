#pragma once

#include "SectionProperty.h"
#include "VirtualProperty.h"

namespace DAVA
{
class UIControl;
class UIComponent;
}

class VarTableValueProperty;

class VirtualPropertiesSection : public SectionProperty<VirtualProperty>
{
public:
    VirtualPropertiesSection(const DAVA::String& name, DAVA::UIComponent* properiesComponent, VarTableValueProperty* properiesProperty);

    DAVA::Any GetLocalValue(const DAVA::String& name);
    void ResetLocalValue(const DAVA::String& propertyName);
    void SetLocalValue(const DAVA::String& name, const DAVA::Any& value);
    bool IsOverriddenLocally(const DAVA::String& name);

    void RebuildVirtualProperties();

    void Accept(PropertyVisitor* visitor) override;
    DAVA::UIComponent* GetComponent();
    bool IsVirtualProperty() const override;

protected:
    virtual ~VirtualPropertiesSection();

private:
    DAVA::UIComponent* properiesComponent; //weakptr
    VarTableValueProperty* properiesProperty;
};
