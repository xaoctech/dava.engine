#pragma once

#include "Model/PackageHierarchy/ControlNode.h"

#include <TArc/DataProcessing/DataNode.h>

class EditorData : public DAVA::TArc::DataNode
{
public:
    ControlNode* GetHighlightedNode() const;

    static DAVA::FastName highlightedNodePropertyName;

private:
    void SetHighlightedNode(ControlNode* node);

    ControlNode* highlightedNode = nullptr;

    DAVA_VIRTUAL_REFLECTION(EditorData, DAVA::TArc::DataNode);
};
