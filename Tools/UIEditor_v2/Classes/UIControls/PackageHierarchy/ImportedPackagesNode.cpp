//
//  ImportedPackagesNode.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.10.14.
//
//

#include "ImportedPackagesNode.h"

#include "PackageControlsNode.h"

using namespace DAVA;

ImportedPackagesNode::ImportedPackagesNode(DAVA::UIPackage *package)
{
    for (int i = 0; i < package->GetPackagesCount(); i++)
    {
        UIPackage *pack = package->GetPackage(i);
        Add(new PackageControlsNode(pack, pack->GetName(), false));
    }
}

ImportedPackagesNode::~ImportedPackagesNode()
{
    
}

String ImportedPackagesNode::GetName() const
{
    return "Imported Packages";
}

