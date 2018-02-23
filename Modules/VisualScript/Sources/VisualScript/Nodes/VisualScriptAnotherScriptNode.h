#pragma once

#include "VisualScript/VisualScriptNode.h"

namespace DAVA
{
class FilePath;
class YamlNode;

#if TODO_VISUAL_ANOTHER_SCRIPT_NODE
class VisualScriptAnotherScriptNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptAnotherScriptNode, VisualScriptNode);

public:
    VisualScriptAnotherScriptNode();
    ~VisualScriptAnotherScriptNode() override = default;

    void SetScriptFilepath(const FilePath& scriptFilepath_);
    const FilePath& GetScriptFilepath() const;

    void BindReflection(const Reflection& ref_) override;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

protected:
    FilePath scriptFilepath;
    VisualScript* anotherScript = nullptr;

    void AssetLoadedCallback(VisualScript* script);
};
#endif
}