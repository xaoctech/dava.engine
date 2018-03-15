#include "VisualScript/Nodes/VisualScriptAnotherScriptNode.h"
#include "VisualScript/VisualScript.h"
#include "VisualScript/VisualScriptPin.h"

#include <Engine/Engine.h>
#include <FileSystem/YamlNode.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptAnotherScriptNode)
{
    ReflectionRegistrator<VisualScriptAnotherScriptNode>::Begin()
    .ConstructorByPointer()
    .End();
}

VisualScriptAnotherScriptNode::VisualScriptAnotherScriptNode()
{
    SetType(SCRIPT);
}

void VisualScriptAnotherScriptNode::SetScriptFilepath(const FilePath& scriptFilepath_)
{
    if (scriptFilepath != scriptFilepath_)
    {
        scriptFilepath = scriptFilepath_;
        SetName(FastName(scriptFilepath.GetBasename()));

        anotherScript = std::make_unique<VisualScript>();
        anotherScript->Load(scriptFilepath);
        anotherScript->Compile();

        // anotherScript = GetEngineContext()->assetManager->LoadAsset<VisualScript>(scriptFilepath, [this](Asset<AssetBase> asset){
        //     DVASSERT(asset == anotherScript);
        //     CompleteScriptLoading();
        // }, false);
        // GetEngineContext()->jobManager->WaitWorkerJobs();

        CompleteScriptLoad();
    }
}

const FilePath& VisualScriptAnotherScriptNode::GetScriptFilepath() const
{
    return scriptFilepath;
}

void VisualScriptAnotherScriptNode::Save(YamlNode* node) const
{
    VisualScriptNode::Save(node);
    node->Add("scriptName", scriptFilepath.GetAbsolutePathname());
    SaveDefaults(node);
}

void VisualScriptAnotherScriptNode::Load(const YamlNode* node)
{
    VisualScriptNode::Load(node);
    FilePath scriptPath(node->Get("scriptName")->AsString());
    SetScriptFilepath(scriptPath);
    LoadDefaults(node);
}

void VisualScriptAnotherScriptNode::CompleteScriptLoad()
{
    const Vector<VisualScriptNode*>& nodes = anotherScript->GetNodes();

    Vector<VisualScriptPin*> allInputPins;
    Vector<VisualScriptPin*> allOutputPins;

    uint32 execInputPins = 0;
    for (const auto& node : nodes)
    {
        const Vector<VisualScriptPin*>& inputPins = node->GetAllInputPins();
        /*
         //
         // More than one exec pin? (is it a problem or not?)
         // For now decided that is not
         //

        for (const auto& inPin: inputPins)
        {
            if (inPin->GetAttribute() == VisualScriptPin::EXEC_IN)
                execInputPins++;
        }*/
        for (const auto& inPin : inputPins)
        {
            if (inPin->GetConnectedSet().size() == 0 && inPin->GetDefaultValue().IsEmpty())
                allInputPins.emplace_back(inPin);
        }

        const Vector<VisualScriptPin*>& outputPins = node->GetAllOutputPins();
        for (const auto& outPin : outputPins)
        {
            if (outPin->GetConnectedSet().size() == 0)
                allOutputPins.emplace_back(outPin);
        }
    }

    /*
        Here we take all open unused pins from script we've loaded and expose them as input & output parameters of our node.
        We copy it's original pins to our node. After that we setup serialization owner. It means that
        when owner will serialize this script, all connections connected to that internal nodes, will be
        treated as connections to this AnotherScriptNode.

        Execution of nodes work automatically because during compilation phase, script compiles as it has
        all nodes included to itself.
     */
    uint32 index = 0;
    for (auto& pin : allInputPins)
    {
        /*
         TODO: We can unify all names in one place. Instead of fragmented code.
         It's only 2-3 places, so not critical right now.
         */
        pin->SetName(FastName(Format("arg%d", index++)));
        RegisterPin(pin);
        pin->SetSerializationOwner(this);
    }

    index = 0;
    for (auto& pin : allOutputPins)
    {
        pin->SetName(FastName(Format("res%d", index++)));
        RegisterPin(pin);
        pin->SetSerializationOwner(this);
    }
}

}