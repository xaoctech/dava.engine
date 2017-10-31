#include "UI/Find/Filters/HasErrorsAndWarningsFilter.h"

#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>

namespace HasErrorsAndWarningsFilterDetails
{
template <typename Component>
const DAVA::ReflectedStructure::Field* FindField(const char* nameStr)
{
    const DAVA::FastName name(nameStr);
    for (const auto& field : DAVA::ReflectedTypeDB::Get<Component>()->GetStructure()->fields)
    {
        if (field->name == name)
        {
            return field.get();
        }
    }
    DVASSERT("can not find field in component!");
    return nullptr;
}

template <typename Type>
Type(const ControlInformation* control, const DAVA::ReflectedStructure::Field* field)
{
    const DAVA::Any& noInput = control->GetControlPropertyValue(*field);
    if (noInput.CanCast<Type>())
    {
        return noInput.Cast<Type>();
    }
    DVASSERT(false, "can not cast field value!");
    return Type();
}
}

HasErrorsAndWarningsFilter::HasErrorsAndWarningsFilter()
{
    using namespace DAVA;

    horizontalSizePolicyField = HasErrorsAndWarningsFilterDetails::FindField<UISizePolicyComponent>("horizontalPolicy");
    verticalSizePolicyField = HasErrorsAndWarningsFilterDetails::FindField<UISizePolicyComponent>("verticalPolicy");

    anchorsEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("enabled");
    leftAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("leftAnchorEnabled");
    hCenterAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("hCenterAnchorEnabled");
    rightAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("rightAnchorEnabled");
    topAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("topAnchorEnabled");
    vCenterAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("vCenterAnchorEnabled");
    bottomAnchorEnabledField = HasErrorsAndWarningsFilterDetails::FindField<UIAnchorComponent>("bottomAnchorEnabled");
}

FindFilter::ePackageStatus HasErrorsAndWarningsFilter::AcceptPackage(const PackageInformation* package) const
{
    return PACKAGE_CAN_ACCEPT_CONTROLS;
}

bool HasErrorsAndWarningsFilter::AcceptControl(const ControlInformation* control) const
{
    using namespace DAVA;

    if (control->HasComponent(Type::Instance<UIAnchorComponent>()) && control->HasComponent(Type::Instance<UISizePolicyComponent>()))
    {
        int32 hSizePolicy = HasErrorsAndWarningsFilterDetails::GetValue<UISizePolicyComponent::eSizePolicy>(control, horizontalSizePolicyField);
        int32 vSizePolicy = HasErrorsAndWarningsFilterDetails::GetValue<UISizePolicyComponent::eSizePolicy>(control, horizontalSizePolicyField);

        bool anchorsEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorsEnabledField);
        bool leftAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, leftAnchorEnabledField);
        bool hCenterAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, hCenterAnchorEnabledField);
        bool rightAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, rightAnchorEnabledField);
        bool topAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, topAnchorEnabledField);
        bool vCenterAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, vCenterAnchorEnabledField);
        bool bottomAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, bottomAnchorEnabledField);
    }
    return false;
}
