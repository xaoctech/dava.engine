#include "UI/Find/Filters/HasErrorsAndWarningsFilter.h"

#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>

#include <Reflection/ReflectedTypeDB.h>

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
Type GetValue(const ControlInformation* control, const DAVA::Type* componentType, const DAVA::ReflectedStructure::Field* field, Type defaultValue)
{
    DVASSERT(DAVA::UIComponent::IsMultiple(componentType) == false);
    const DAVA::Any& value = control->GetComponentPropertyValue(componentType, 0, *field);
    if (value.CanCast<Type>())
    {
        return value.Cast<Type>();
    }
    return defaultValue;
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
    const Type* anchorComponentType = Type::Instance<UIAnchorComponent>();
    const Type* sizePolicyComponentType = Type::Instance<UISizePolicyComponent>();

    if (control->HasComponent(anchorComponentType) && control->HasComponent(sizePolicyComponentType))
    {
        UISizePolicyComponent::eSizePolicy hSizePolicy = HasErrorsAndWarningsFilterDetails::GetValue<UISizePolicyComponent::eSizePolicy>(control, sizePolicyComponentType, horizontalSizePolicyField, UISizePolicyComponent::eSizePolicy::IGNORE_SIZE);
        UISizePolicyComponent::eSizePolicy vSizePolicy = HasErrorsAndWarningsFilterDetails::GetValue<UISizePolicyComponent::eSizePolicy>(control, sizePolicyComponentType, verticalSizePolicyField, UISizePolicyComponent::eSizePolicy::IGNORE_SIZE);

        bool anchorsEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, anchorsEnabledField, true);
        bool leftAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, leftAnchorEnabledField, false);
        bool hCenterAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, hCenterAnchorEnabledField, false);
        bool rightAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, rightAnchorEnabledField, false);
        bool topAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, topAnchorEnabledField, false);
        bool vCenterAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, vCenterAnchorEnabledField, false);
        bool bottomAnchorEnabled = HasErrorsAndWarningsFilterDetails::GetValue<bool>(control, anchorComponentType, bottomAnchorEnabledField, false);

        if (anchorsEnabled == false)
        {
            return false;
        }

        if (hSizePolicy != UISizePolicyComponent::IGNORE_SIZE && (leftAnchorEnabled || hCenterAnchorEnabled || rightAnchorEnabled))
        {
            return true;
        }

        if (vSizePolicy != UISizePolicyComponent::IGNORE_SIZE && (topAnchorEnabled || vCenterAnchorEnabled || bottomAnchorEnabled))
        {
            return true;
        }
    }
    return false;
}
