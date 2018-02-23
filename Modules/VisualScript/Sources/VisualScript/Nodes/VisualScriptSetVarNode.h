#pragma once

#include "VisualScript/VisualScriptNode.h"
#include <Base/FastName.h>

namespace DAVA
{
class YamlNode;

class VisualScriptSetVarNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptSetVarNode, VisualScriptNode);

public:
    VisualScriptSetVarNode();
    VisualScriptSetVarNode(const Reflection& ref, const FastName& varPath);
    ~VisualScriptSetVarNode() override = default;

    void SetVarPath(const FastName& varPath_);
    const FastName& GetVarPath() const;

    void BindReflection(const Reflection& ref) override;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    FastName varPath;
    VisualScriptPin* varInPin = nullptr;
    VisualScriptPin* varOutPin = nullptr;
};
}
