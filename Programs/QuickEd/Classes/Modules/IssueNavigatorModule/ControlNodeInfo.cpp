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

bool IsRootControl(const PackageNode* package, const ControlNode* node)
{
    return ((node->GetParent() == package->GetPackageControlsNode()) || (node->GetParent() == package->GetPrototypes()));
}

DAVA::String GetPathToControl(const PackageNode* package, const ControlNode* node)
{
    using namespace DAVA;
    using namespace Detail;

    DAVA::String pathToControl = node->GetName();
    for (const ControlNode* nextNode = GetParentNode(package, node);
         nextNode != nullptr;
         nextNode = GetParentNode(package, nextNode))
    {
        pathToControl = nextNode->GetName() + "/" + pathToControl;
    }

    return pathToControl;
}
}