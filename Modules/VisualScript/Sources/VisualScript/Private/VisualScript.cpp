#include "VisualScript/VisualScript.h"
#include "VisualScript/Nodes/VisualScriptEventNode.h"
#include "VisualScript/Nodes/VisualScriptGetVarNode.h"
#include "VisualScript/Nodes/VisualScriptSetVarNode.h"
#include "VisualScript/Nodes/VisualScriptFunctionNode.h"
#include "VisualScript/VisualScriptPin.h"
#include "VisualScript/VisualScriptEvents.h"

#include "FileSystem/YamlEmitter.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/Utils.h"
#include "Math/Vector.h"
#include "Math/Matrix2.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Render/Image/Image.h"
#include "Utils/StringUtils.h"

#include <algorithm>

namespace DAVA
{
//namespace VisualScriptDetails
//{
//const ReflectedType* GetValueReflectedType(const Any& value);
//
//const ReflectedType* GetValueReflectedType(const Reflection& r)
//{
//    const ReflectedType* type = GetValueReflectedType(r.GetValue());
//    if (type != nullptr)
//    {
//        return type;
//    }
//
//    return r.GetValueObject().GetReflectedType();
//}
//
//const ReflectedType* GetValueReflectedType(const Any& value)
//{
//    if (value.IsEmpty() == true)
//    {
//        return nullptr;
//    }
//
//    const Type* type = value.GetType();
//    if (type->IsPointer())
//    {
//        void* pointerValue = value.Get<void*>();
//        if (pointerValue != nullptr)
//        {
//            return ReflectedTypeDB::GetByPointer(pointerValue, type->Deref());
//        }
//        else
//        {
//            return ReflectedTypeDB::GetByType(type->Deref());
//        }
//    }
//
//    return ReflectedTypeDB::GetByType(type);
//}
//}

DAVA_REFLECTION_IMPL(VisualScript)
{
    ReflectionRegistrator<VisualScript>::Begin()
    .Field("nodes", &VisualScript::nodes)[M::Serialize(), M::HiddenField()]
    .Field("dataRegistry", &VisualScript::dataRegistry)[M::MutableCollection(), M::DisplayName("Data Registry")]
    .End();
};

VisualScript::VisualScript()
{
    defaultValues[FastName("bool")] = bool();
    defaultValues[FastName("int32")] = int32();
    defaultValues[FastName("uint32")] = uint32();
    defaultValues[FastName("float32")] = float32();
    defaultValues[FastName("float64")] = float64();
    defaultValues[FastName("String")] = String();
    defaultValues[FastName("WideString")] = WideString();
    defaultValues[FastName("FastName")] = FastName();
    defaultValues[FastName("Color")] = Color();
    defaultValues[FastName("Vector2")] = Vector2();
    defaultValues[FastName("Vector3")] = Vector3();
    defaultValues[FastName("Vector4")] = Vector4();
    defaultValues[FastName("Matrix2")] = Matrix2();
    defaultValues[FastName("Matrix3")] = Matrix3();
    defaultValues[FastName("Matrix4")] = Matrix4();
    defaultValues[FastName("Image")] = static_cast<Image*>(nullptr);
}

VisualScript::~VisualScript()
{
    ReleaseScript();
}

void VisualScript::ReleaseScript()
{
    for (auto& node : nodes)
    {
        SafeDelete(node);
    }
    nodes.clear();
    eventNodes.clear();
    connections.clear();
    nameToNode.clear();
}

void VisualScript::RemoveNode(VisualScriptNode* node)
{
    DVASSERT(node->GetAllConnections().empty() == true);
    FindAndRemoveExchangingWithLast(nodes, node);

    SafeDelete(node);
}

void VisualScript::Save(const FilePath& filepath)
{
    ScopedPtr<YamlNode> rootNode(YamlNode::CreateMapNode());

    // Store nodes and connections
    YamlNode* yAllNodes = YamlNode::CreateMapNode();
    rootNode->Add("nodes", yAllNodes);
    if (!nodes.empty())
    {
        Map<VisualScriptNode*, String> uniqueNames;

        int32 uniqueTypeIndices[VisualScriptNode::TYPE_COUNT];
        for (uint32 k = 0; k < VisualScriptNode::TYPE_COUNT; ++k)
            uniqueTypeIndices[k] = 1;

        for (auto& node : nodes)
        {
            const ReflectedType* reflectedType = ReflectedTypeDB::GetByPointer(node);

            String name = Format("%s%d", reflectedType->GetPermanentName().c_str(), uniqueTypeIndices[node->GetType()]++);
            uniqueNames.emplace(node, name);
        }

        VisualScriptNode* node = nodes[0];
        Reflection ref = Reflection::Create(node);
        const Type* type = ref.GetValueType();

        uint32 uniqueIndex = 1;

        for (auto& node : nodes)
        {
            auto connections = node->GetAllConnections();

            const ReflectedType* reflectedType = ReflectedTypeDB::GetByPointer(node);

            Logger::Debug("uname:%s type: %s pname: %s", uniqueNames[node].c_str(), type->GetDemangledName().c_str(), reflectedType->GetPermanentName().c_str());

            YamlNode* yNode = YamlNode::CreateMapNode();
            yAllNodes->Add(uniqueNames[node].c_str(), yNode);

            yNode->Add("name", uniqueNames[node].c_str());

            DVASSERT(reflectedType != nullptr);
            DVASSERT(reflectedType->GetPermanentName() != "");

            yNode->Add("type", reflectedType->GetPermanentName().c_str());

            YamlNode* yNodeInternal = YamlNode::CreateMapNode();
            node->Save(yNodeInternal);
            yNode->Add("node", yNodeInternal);

            auto yConnectionsNode = YamlNode::CreateArrayNode(YamlNode::AR_BLOCK_REPRESENTATION);
            yNode->Add("connections", yConnectionsNode);
            for (const auto& connection : connections)
            {
                VisualScriptPin* pinIn = connection.first;
                VisualScriptPin* pinOut = connection.second;
                VisualScriptNode* otherNode = pinOut->GetSerializationOwner();

                Logger::Debug("%s.%s -> %s.%s", uniqueNames[node].c_str(),
                              pinIn->GetName().c_str(),
                              uniqueNames[otherNode].c_str(),
                              pinOut->GetName().c_str());

                auto yOneConnectionNode = YamlNode::CreateMapNode();
                yConnectionsNode->Add(yOneConnectionNode);
                auto yInConnectionNode = YamlNode::CreateMapNode();
                auto yOutConnectionNode = YamlNode::CreateMapNode();
                yOneConnectionNode->Add("in", yInConnectionNode);
                yOneConnectionNode->Add("out", yOutConnectionNode);
                yInConnectionNode->Add("name", uniqueNames[node].c_str());
                yInConnectionNode->Add("pin", pinIn->GetName().c_str());
                yOutConnectionNode->Add("name", uniqueNames[otherNode].c_str());
                yOutConnectionNode->Add("pin", pinOut->GetName().c_str());
            }
            uniqueIndex++;
        };
    }

    // Store variables
    YamlNode* variablesNode = YamlNode::CreateMapNode();
    rootNode->Add("variables", variablesNode);
    for (auto it : dataRegistry)
    {
        const FastName& name = it.first;
        const Any& value = it.second;
        const Type* type = value.GetType();
        const ReflectedType* reflType = ReflectedTypeDB::GetByType(type);
        DVASSERT(reflType->GetPermanentName() != "");

        YamlNode* varNode = YamlNode::CreateArrayNode();
        YamlNode* varDataNode = YamlNode::CreateNodeFromAny(value);

        varNode->Add(reflType->GetPermanentName());
        varNode->Add(varDataNode);
        variablesNode->Add(name.c_str(), varNode);
    }

    YamlEmitter::SaveToYamlFile(filepath, rootNode);
}

void VisualScript::PrepareDataForReload(const FilePath& filepath)
{
    DVASSERT(reloadContext == nullptr);

    reloadContext = new LoadContext();
    reloadContext->Load(this, filepath);
}

void VisualScript::Reload()
{
    ReleaseScript();

    reloadContext->MoveToScript(this);
    SafeDelete(reloadContext);

    Compile();
}

void VisualScript::Load(const FilePath& filepath)
{
    LoadContext loadContext;
    loadContext.Load(this, filepath);
    loadContext.MoveToScript(this);

    Compile();
}

void VisualScript::LoadContext::MoveToScript(VisualScript* script)
{
    script->nodes = nodes;
    script->nameToNode = nameToNode;
    script->connections = connections;
    script->dataRegistry = dataRegistry;
}

void VisualScript::LoadContext::Load(VisualScript* script, const FilePath& filepath)
{
    Logger::Debug("VisualScript::Load(%s)", filepath.GetStringValue().c_str());
    ScopedPtr<YamlParser> parser(YamlParser::Create(filepath));

    // Create nodes
    YamlNode* rootNode = parser->GetRootNode();

    const YamlNode* nodes = rootNode->Get("nodes");
    for (uint32 k = 0; k < nodes->GetCount(); ++k)
    {
        const YamlNode* node = nodes->Get(k);

        const YamlNode* nodeName = node->Get("name");
        const YamlNode* nodeType = node->Get("type");

        const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(nodeType->AsString());
        VisualScriptNode* createdNode = script->CreateNodeWithoutAdd(reflectedType);
        this->nodes.push_back(createdNode);

        nameToNode.emplace(FastName(nodeName->AsString()), createdNode);

        const YamlNode* nodeInternal = node->Get("node");
        createdNode->Load(nodeInternal);
    }

    // Restore connections
    for (uint32 k = 0; k < nodes->GetCount(); ++k)
    {
        const YamlNode* node = nodes->Get(k);
        // connections
        const YamlNode* yConnections = node->Get("connections");

        uint32 connectionCount = yConnections->GetCount();
        for (uint32 connectionIndex = 0; connectionIndex < connectionCount; ++connectionIndex)
        {
            const YamlNode* connection = yConnections->Get(connectionIndex);
            const YamlNode* in = connection->Get("in");
            const YamlNode* out = connection->Get("out");

            const YamlNode* inPin = in->Get("pin");
            const YamlNode* inNodeName = in->Get("name");
            const YamlNode* outPin = out->Get("pin");
            const YamlNode* outNodeName = out->Get("name");

            Connection con =
            {
              { FastName(inNodeName->AsString()), FastName(inPin->AsString()) },
              { FastName(outNodeName->AsString()), FastName(outPin->AsString()) },
            };

            connections.emplace_back(con);
        }
    }

    // Variables
    const YamlNode* variables = rootNode->Get("variables");
    for (uint32 k = 0; k < variables->GetCount(); ++k)
    {
        FastName varName = FastName(variables->GetItemKeyName(k));
        const YamlNode* var = variables->Get(k);
        const String typeName = var->Get(0)->AsString();
        DVASSERT(typeName != "");
        const Type* type = ReflectedTypeDB::GetByPermanentName(typeName)->GetType();
        Any value = var->Get(1)->AsAny(type);
        dataRegistry.emplace(varName, value);
    }
}

VisualScriptNode* VisualScript::FindNodeByName(const FastName& name)
{
    auto it = nameToNode.find(name);
    if (it != nameToNode.end())
    {
        return it->second;
    }
    return nullptr;
}

void VisualScript::Compile()
{
    Reflection variablesReflection = Reflection::Create(ReflectedObject(&dataRegistry));

    eventNodes.clear();
    for (auto node : nodes)
    {
        switch (node->GetType())
        {
        case VisualScriptNode::EVENT:
        {
            VisualScriptEventNode* eventNode = static_cast<VisualScriptEventNode*>(node);
            const FastName& eventName = eventNode->GetEventName();
            eventNodes.emplace(eventName, eventNode);
            break;
        }
        case VisualScriptNode::GET_VAR:
        {
            VisualScriptGetVarNode* varNode = static_cast<VisualScriptGetVarNode*>(node);
            varNode->BindReflection(variablesReflection);
            break;
        }
        case VisualScriptNode::SET_VAR:
        {
            VisualScriptSetVarNode* varNode = static_cast<VisualScriptSetVarNode*>(node);
            varNode->BindReflection(variablesReflection);
            break;
        }
        default:
            // Skip
            break;
        }
    }

    for (auto& connection : connections)
    {
        VisualScriptNode* inNode = FindNodeByName(connection.in.nodeName);
        VisualScriptNode* outNode = FindNodeByName(connection.out.nodeName);
        if (inNode && outNode)
        {
            VisualScriptPin* inPin = inNode->GetPinByName(connection.in.pinName);
            VisualScriptPin* outPin = outNode->GetPinByName(connection.out.pinName);

            // Legacy support (pin names like `argN`)
            if (inPin == nullptr)
            {
                if (StringUtils::StartsWith(connection.in.pinName.c_str(), "arg"))
                {
                    int32 index = atoi(connection.in.pinName.c_str() + 3);
                    inPin = inNode->GetDataInputPin(index);
                    if (inPin)
                    {
                        Logger::Warning("Find input pin by order %s.%s = %s.%s",
                                        connection.in.nodeName.c_str(),
                                        connection.in.pinName.c_str(),
                                        connection.in.nodeName.c_str(),
                                        inPin->GetName().c_str());
                    }
                }
            }
            if (outPin == nullptr)
            {
                if (StringUtils::StartsWith(connection.out.pinName.c_str(), "arg"))
                {
                    int32 index = atoi(connection.out.pinName.c_str() + 3);
                    outPin = outNode->GetDataInputPin(index);
                    if (outPin)
                    {
                        Logger::Warning("Find output pin by order %s.%s = %s.%s",
                                        connection.out.nodeName.c_str(),
                                        connection.out.pinName.c_str(),
                                        connection.out.nodeName.c_str(),
                                        outPin->GetName().c_str());
                    }
                }
            }
            // End legacy support

            if (inPin && outPin)
            {
                inPin->Connect(outPin);
            }
            else
            {
                if (!inPin)
                {
                    Logger::Error("Input pin not found: %s.%s", connection.in.nodeName.c_str(), connection.in.pinName.c_str());
                }
                if (!outPin)
                {
                    Logger::Error("Output pin not found: %s.%s", connection.out.nodeName.c_str(), connection.out.pinName.c_str());
                }
            }
        }
    }

    //
    // TODO: Go by all events nodes and recompile everything
    //
}

void VisualScript::Execute(const FastName& eventNodeName, const Reflection& eventReflection)
{
    Logger::Debug("VisualScript::Execute(%s)", eventNodeName.c_str());
    auto it = eventNodes.find(eventNodeName);
    if (it == eventNodes.end())
        return;

    VisualScriptEventNode* eventNode = it->second;
    eventNode->BindReflection(eventReflection);
    VisualScriptPin* entryPin = eventNode->GetExecOutputPin(0)->GetConnectedTo();

    if (recompileScriptOnExecute)
    {
        try
        {
            executor.Compile(entryPin);
        }
        catch (Exception& exception)
        {
            DAVA::Logger::Error(exception.what());
        }
    }
    executor.Execute(entryPin);

    Logger::Debug("VisualScript::Execute finished");
}

const Vector<VisualScriptNode*>& VisualScript::GetNodes() const
{
    return nodes;
}

void VisualScript::SetRecompileOnExecute(bool recompileScriptOnExecute_)
{
    recompileScriptOnExecute = recompileScriptOnExecute_;
}
}
