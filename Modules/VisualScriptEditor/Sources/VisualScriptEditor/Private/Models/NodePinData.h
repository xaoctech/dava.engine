#pragma once

#include <nodes/NodeData>

#include <VisualScript/VisualScriptNode.h>

namespace DAVA
{
class VisualScriptPin;
class NodePinData : public QtNodes::NodeData
{
public:
    NodePinData(VisualScriptPin* pin);
    QtNodes::NodeDataType type() const override;

    VisualScriptPin* GetPin() const;

private:
    VisualScriptPin* pin = nullptr;
};

} // DAVA
