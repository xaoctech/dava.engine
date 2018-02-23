#pragma once

#include "VisualScript/VisualScriptNode.h"
#include <Base/AnyFn.h>
#include <Base/FastName.h>

namespace DAVA
{
class YamlNode;

class VisualScriptFunctionNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptFunctionNode, VisualScriptNode);

public:
    VisualScriptFunctionNode();
    VisualScriptFunctionNode(const FastName& className_, const FastName& functionName_);
    ~VisualScriptFunctionNode() override = default;

    void SetClassName(const FastName& className);
    void SetFunctionName(const FastName& functionName);

    const FastName& GetClassName() const;
    const FastName& GetFunctionName() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    void InitPins();
    void InitNodeWithAnyFn(const AnyFn& function);

    FastName className;
    FastName functionName;
};
}
