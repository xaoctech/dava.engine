#pragma once

#include "VisualScript/VisualScriptNode.h"

#include <Base/FastName.h>

namespace DAVA
{
class VisualScriptEventNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptEventNode, VisualScriptNode);

public:
    VisualScriptEventNode();
    VisualScriptEventNode(const FastName& eventName_);
    ~VisualScriptEventNode() override = default;

    void SetEventName(const FastName& eventName);
    const FastName& GetEventName() const;

    void BindReflection(const Reflection& ref);

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

protected:
    FastName eventName;
};
}
