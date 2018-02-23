#pragma once

#include "Base/FastName.h"
#include "ValueProperty.h"

class ControlNode;
class VirtualPropertiesSection;

class VirtualProperty : public ValueProperty
{
public:
    VirtualProperty(VirtualPropertiesSection* vSection, const DAVA::String& propertyName, const DAVA::Type* type);

protected:
    virtual ~VirtualProperty();

public:
    void Refresh(DAVA::int32 refreshFlags) override;
    void Accept(PropertyVisitor* visitor) override;

    DAVA::uint32 GetFlags() const override;

    bool IsReadOnly() const override;

    ePropertyType GetType() const override;
    DAVA::Any GetValue() const override;

    //bool IsOverridden() const override;
    bool IsOverriddenLocally() const override;

    void ResetValue() override;
    bool IsVirtualProperty() const override;

protected:
    void ApplyValue(const DAVA::Any& value) override;

protected:
    DAVA::String propertyName;
    VirtualPropertiesSection* vSection;
};
