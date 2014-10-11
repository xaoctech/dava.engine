//
//  PackageNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#include "PackageNode.h"

#include "PackageControlsNode.h"
#include "ImportedPackagesNode.h"

using namespace DAVA;

PackageNode::PackageNode(DAVA::UIPackage *package) : package(SafeRetain(package))
{
    Add(new ImportedPackagesNode(package));
    Add(new PackageControlsNode(package, package->GetName(), true));
}

PackageNode::~PackageNode()
{
    SafeRelease(package);
}

String PackageNode::GetName() const
{
    return package->GetName();
}

