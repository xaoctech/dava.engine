#pragma once

#include "Classes/UI/Find/PackageInformation/ControlInformation.h"
#include "Classes/UI/Find/Filters/Private/FieldHolder.h"

#include <UI/Layouts/UISizePolicyComponent.h>
#include <UI/Layouts/UIAnchorComponent.h>

class AnchorsSizePoliciesChecker
{
public:
    AnchorsSizePoliciesChecker();

    bool HasConflicts(const ControlInformation* control) const;

private:
    FieldHolder<DAVA::UISizePolicyComponent, DAVA::UISizePolicyComponent::eSizePolicy> horizontalSizePolicyHolder;
    FieldHolder<DAVA::UISizePolicyComponent, DAVA::UISizePolicyComponent::eSizePolicy> verticalSizePolicyHolder;

    FieldHolder<DAVA::UIAnchorComponent, bool> anchorsEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> leftAnchorEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> hCenterAnchorEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> rightAnchorEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> topAnchorEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> vCenterAnchorEnabledHolder;
    FieldHolder<DAVA::UIAnchorComponent, bool> bottomAnchorEnabledHolder;
};