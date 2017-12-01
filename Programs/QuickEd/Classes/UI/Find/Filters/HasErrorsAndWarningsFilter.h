#pragma once

#include "UI/Find/Filters/FindFilter.h"

#include <Reflection/ReflectedStructure.h>

class HasErrorsAndWarningsFilter : public FindFilter
{
public:
    HasErrorsAndWarningsFilter();

private:
    FindFilter::ePackageStatus AcceptPackage(const PackageInformation* package) const override;
    bool AcceptControl(const ControlInformation* control) const override;

    const DAVA::ReflectedStructure::Field* horizontalSizePolicyField = nullptr;
    const DAVA::ReflectedStructure::Field* verticalSizePolicyField = nullptr;

    const DAVA::ReflectedStructure::Field* anchorsEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* leftAnchorEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* hCenterAnchorEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* rightAnchorEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* topAnchorEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* vCenterAnchorEnabledField = nullptr;
    const DAVA::ReflectedStructure::Field* bottomAnchorEnabledField = nullptr;
};
