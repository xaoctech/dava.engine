#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

class PackageNode;
class ControlNode;

namespace ControlNodeInfo
{
bool IsRootControl(const PackageNode* package, const ControlNode* node);
DAVA::String GetPathToControl(const PackageNode* package, const ControlNode* node);
}