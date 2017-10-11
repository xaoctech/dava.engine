#pragma once

#include "Model/PackageHierarchy/ControlNode.h"

#include <TArc/DataProcessing/DataNode.h>

class HUDSystem;

class HUDModuleData : public DAVA::TArc::DataNode
{
public:
    ~HUDModuleData() override;

    static DAVA::FastName highlightedNodePropertyName;

    ControlNode* GetHighlight() const;

private:
    friend class HUDModule;
    std::unique_ptr<HUDSystem> hudSystem;
    ControlNode* highlightedNode = nullptr;

    DAVA_VIRTUAL_REFLECTION(HUDModuleData, DAVA::TArc::DataNode);
};
