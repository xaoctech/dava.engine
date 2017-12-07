#pragma once

#include "UI/Find/Filters/FindFilter.h"

#include <memory>

class AnchorsSizePoliciesChecker;

class HasErrorsAndWarningsFilter : public FindFilter
{
public:
    HasErrorsAndWarningsFilter();
    ~HasErrorsAndWarningsFilter();

private:
    FindFilter::ePackageStatus AcceptPackage(const PackageInformation* package) const override;
    bool AcceptControl(const ControlInformation* control) const override;

    std::unique_ptr<AnchorsSizePoliciesChecker> anchorsSizePoliciesHolder;
};
