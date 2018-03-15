#pragma once

#include "VisualScript/VisualScriptNode.h"
#include <FileSystem/FilePath.h>

namespace DAVA
{
class YamlNode;
class VisualScript;

class VisualScriptAnotherScriptNode : public VisualScriptNode
{
    DAVA_VIRTUAL_REFLECTION(VisualScriptAnotherScriptNode, VisualScriptNode);

public:
    VisualScriptAnotherScriptNode();
    ~VisualScriptAnotherScriptNode() override = default;

    void SetScriptFilepath(const FilePath& scriptFilepath_);
    const FilePath& GetScriptFilepath() const;

    void Save(YamlNode* node) const override;
    void Load(const YamlNode* node) override;

protected:
    FilePath scriptFilepath;
    std::unique_ptr<VisualScript> anotherScript;

    void CompleteScriptLoad();
};
}