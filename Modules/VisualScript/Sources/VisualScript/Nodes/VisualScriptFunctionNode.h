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
    VisualScriptFunctionNode(const FastName& className, const FastName& functionName);
    ~VisualScriptFunctionNode() override = default;

    void SetClassName(const FastName& className);
    const FastName& GetClassName() const;
    void SetFunctionName(const FastName& functionName);
    const FastName& GetFunctionName() const;

    const AnyFn& GetFunction() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

private:
    void InitPins();

    FastName className;
    FastName functionName;
    AnyFn function;
};
}
