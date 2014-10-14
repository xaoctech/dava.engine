#include "PackageControlsNode.h"

#include "ControlNode.h"

using namespace DAVA;

PackageControlsNode::PackageControlsNode(PackageBaseNode *parent, UIPackage *package) : PackageBaseNode(parent), name("Controls"), editable(true), package(SafeRetain(package))
{
}

PackageControlsNode::~PackageControlsNode()
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        (*it)->Release();
    nodes.clear();
    SafeRelease(package);
}

void PackageControlsNode::Add(ControlNode *node)
{
    DVASSERT(node->GetParent() == NULL);
    node->SetParent(this);
    nodes.push_back(SafeRetain(node));
}

int PackageControlsNode::GetCount() const
{
    return (int) nodes.size();
}

ControlNode *PackageControlsNode::Get(int index) const
{
    return nodes[index];
}

String PackageControlsNode::GetName() const
{
    return name;
}

void PackageControlsNode::SetName(const DAVA::String &name)
{
    this->name = name;
}

UIPackage *PackageControlsNode::GetPackage() const
{
    return package;
}

const FilePath &PackageControlsNode::GetPackagePath() const
{
    return package->GetFilePath();
}

bool PackageControlsNode::IsInstancedFromPrototype() const
{
    return false;
}

bool PackageControlsNode::IsCloned() const
{
    return false;
}

ControlNode *PackageControlsNode::FindControlNodeByName(const DAVA::String &name) const
{
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
    {
        if ((*it)->GetControl()->GetName() == name)
            return *it;
    }
    return NULL;
}

YamlNode *PackageControlsNode::Serialize() const
{
    YamlNode *arrayNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
    for (auto it = nodes.begin(); it != nodes.end(); ++it)
        arrayNode->Add((*it)->Serialize(NULL));
    return arrayNode;
}
