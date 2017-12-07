#include "UI/Find/Filters/HasErrorsAndWarningsFilter.h"
#include "UI/Find/Filters/Private/AnchorsAndSizePoliciesChecker.h"

#include "UI/Find/PackageInformation/ControlInformation.h"
#include "UI/Find/PackageInformation/PackageInformation.h"

#include <Reflection/ReflectedTypeDB.h>

HasErrorsAndWarningsFilter::HasErrorsAndWarningsFilter()
    : anchorsSizePoliciesHolder(new AnchorsSizePoliciesChecker())
{
}

HasErrorsAndWarningsFilter::~HasErrorsAndWarningsFilter()
{
}

FindFilter::ePackageStatus HasErrorsAndWarningsFilter::AcceptPackage(const PackageInformation* package) const
{
    return PACKAGE_CAN_ACCEPT_CONTROLS;
}

bool HasErrorsAndWarningsFilter::AcceptControl(const ControlInformation* control) const
{
    AnchorsSizePoliciesChecker holder;
    return holder.HasConflicts(control);
}
