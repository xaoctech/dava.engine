#include "Classes/Modules/IssueNavigatorModule/ControlNodeInfo.h"

#include "Classes/Model/ControlProperties/NameProperty.h"
#include "Classes/Model/ControlProperties/RootProperty.h"
#include "Classes/Model/PackageHierarchy/PackageNode.h"
#include "Classes/Model/PackageHierarchy/PackageControlsNode.h"
#include "Classes/Model/PackageHierarchy/ControlNode.h"

namespace ControlNodeInfo
{
namespace Detail
{
ControlNode* GetParentNode(const PackageNode* package, const ControlNode* node)
{
    return IsRootControl(package, node) ? nullptr : dynamic_cast<ControlNode*>(node->GetParent());
};
} // namespace ControlNodeInfoDetail

DAVA::FastName GetNameFromProperty(const ControlNode* node)
{
    NameProperty* nameProperty = node->GetRootProperty()->GetNameProperty();
    return nameProperty->GetValue().Cast<DAVA::FastName>();
}

bool IsRootControl(const PackageNode* package, const ControlNode* node)
{
    return ((node->GetParent() == package->GetPackageControlsNode()) || (node->GetParent() == package->GetPrototypes()));
}

DAVA::String GetPathToControl(const PackageNode* package, const ControlNode* node)
{
    using namespace DAVA;
    using namespace Detail;

    FastName name = GetNameFromProperty(node);
    String nameString = name.c_str();
    DAVA::String pathToControl = nameString;

    for (const ControlNode* nextNode = GetParentNode(package, node);
         nextNode != nullptr;
         nextNode = GetParentNode(package, nextNode))
    {
        String parentName = GetNameFromProperty(nextNode).c_str();
        pathToControl = parentName + "/" + pathToControl;
    }

    return pathToControl;
}
}