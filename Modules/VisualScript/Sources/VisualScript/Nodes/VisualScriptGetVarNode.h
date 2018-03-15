#pragma once

#include "VisualScript/VisualScriptNode.h"
#include <Base/FastName.h>

namespace DAVA
{
class YamlNode;

class VisualScriptGetVarNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptGetVarNode, VisualScriptNode);

public:
    VisualScriptGetVarNode();
    VisualScriptGetVarNode(const Reflection& ref, const FastName& varPath);
    ~VisualScriptGetVarNode() override = default;

    void SetVarPath(const FastName& varPath);
    const FastName& GetVarPath() const;

    void BindReflection(const Reflection& ref);
    const Reflection& GetReflection() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    FastName varPath;
    Reflection reflection;
    VisualScriptPin* outValuePin = nullptr;
};
}
