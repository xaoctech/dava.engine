#pragma once

#include "IntrospectionProperty.h"

class VarTableValueProperty : public IntrospectionProperty
{
public:
    VarTableValueProperty(DAVA::BaseObject* object, const DAVA::String& name, const DAVA::Reflection& ref, const IntrospectionProperty* sourceProperty, eCloneType copyType);
    ~VarTableValueProperty() override;

    DAVA::Any GetDefaultLocalValue(const DAVA::String& propertyName);
    DAVA::Any GetLocalValue(const DAVA::String& name) const;
    void ResetLocalValue(const DAVA::String& propertyName);
    void SetLocalValue(const DAVA::String& name, const DAVA::Any& value);
    bool IsOverriddenLocally(const DAVA::String& name);
    void UpdateRealProperties();

    void SetValue(const DAVA::Any& newValue) override;
    void SetDefaultValue(const DAVA::Any& newValue) override;
    void ResetValue() override;
};
