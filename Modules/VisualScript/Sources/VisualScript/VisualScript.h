#pragma once
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/FastName.h"
#include "Logger/Logger.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/KeyedArchiveAny.h"
#include "Reflection/ReflectedTypeDB.h"
#include "VisualScript/VisualScriptNode.h"
#include "VisualScript/VisualScriptExecutor.h"
#include "Asset/AssetBase.h"

namespace DAVA
{
struct VSFastNameLess
{
    bool operator()(const FastName& v1, const FastName& v2) const
    {
        bool v1Empty = v1.empty();
        bool v2Empty = v2.empty();

        if (v1Empty == true && v2Empty == true)
        {
            return false;
        }

        if (v1Empty == true && v2Empty == false)
        {
            return false;
        }

        if (v1Empty == false && v2Empty == true)
        {
            return true;
        }

        const char* str1 = v1.c_str();
        const char* str2 = v2.c_str();
        return strcmp(str1, str2) < 0;
    }
};

class VisualScript : public AssetBase
{
public:
    DAVA_VIRTUAL_REFLECTION(VisualScript, AssetBase);

    VisualScript();
    ~VisualScript();

    template <typename T, typename... Args>
    T* CreateNode(Args&&... args);

    template <typename... Args>
    VisualScriptNode* CreateNode(const ReflectedType* reflectedType, Args&&... args);

    void RemoveNode(VisualScriptNode* node);

    const Vector<VisualScriptNode*>& GetNodes() const;

    void Compile();
    void Execute(const FastName& eventNodeName, const Reflection& eventReflection);

    void Save(const FilePath& filepath) override;
    void Load(const FilePath& filepath) override;
    void PrepareDataForReload(const FilePath& filepath) override;
    void Reload() override;

    void SetRecompileOnExecute(bool recompileScriptOnExecute_);

    VisualScriptNode* FindNodeByName(const FastName& name);

private:
    template <typename... Args>
    VisualScriptNode* CreateNodeWithoutAdd(const ReflectedType* reflectedType, Args&&... args);
    void ReleaseScript();

    struct Connection
    {
        struct Pin
        {
            FastName nodeName;
            FastName pinName;
        };

        Pin in;
        Pin out;
    };

    struct FastNameValueLess; /*TODO: Move to FastName.h */

    UnorderedMap<FastName, VisualScriptEventNode*> eventNodes;
    Vector<VisualScriptNode*> nodes;

    Vector<Connection> connections;
    UnorderedMap<FastName, VisualScriptNode*> nameToNode;

    VisualScriptExecutor executor;
    bool recompileScriptOnExecute = true;

    struct LoadContext
    {
        void Load(VisualScript* script, const FilePath& filepath);
        void MoveToScript(VisualScript* script);

        Vector<VisualScriptNode*> nodes;
        UnorderedMap<FastName, VisualScriptNode*> nameToNode;
        Vector<Connection> connections;

        Map<FastName, Any, VSFastNameLess> dataRegistry;
    };
    LoadContext* reloadContext = nullptr;

public:
    Map<FastName, Any, VSFastNameLess> dataRegistry;
    Map<FastName, Any, VSFastNameLess> defaultValues;

    friend struct LoadContext;
};

template <typename T, typename... Args>
T* VisualScript::CreateNode(Args&&... args)
{
    return static_cast<T*>(CreateNode(ReflectedTypeDB::Get<T>(), std::forward<Args>(args)...));
}

template <typename... Args>
VisualScriptNode* VisualScript::CreateNode(const ReflectedType* reflectedType, Args&&... args)
{
    VisualScriptNode* node = CreateNodeWithoutAdd(reflectedType, std::forward<Args>(args)...);
    if (node)
        nodes.push_back(node);
    return node;
}

template <typename... Args>
VisualScriptNode* VisualScript::CreateNodeWithoutAdd(const ReflectedType* reflectedType, Args&&... args)
{
    try
    {
        const Type* type = reflectedType->GetType()->Pointer();
        Any object = reflectedType->CreateObject(ReflectedType::CreatePolicy::ByPointer, std::forward<Args>(args)...);
        VisualScriptNode* ptr = object.Cast<VisualScriptNode*>();
        return ptr;
    }
    catch (Exception& exception)
    {
        Logger::Error(exception.what());
        return nullptr;
    }
}
}
