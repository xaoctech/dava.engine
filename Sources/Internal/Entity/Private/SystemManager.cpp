#include "Entity/SystemManager.h"

#include "Base/AnyFn.h"
#include "Entity/SceneSystem.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "UI/UISystem.h"

namespace DAVA
{
namespace SystemManagerDetails
{
struct SysTypeToMethod
{
    SysTypeToMethod(const Type* type_, const ReflectedStructure::Method* method_)
        : type(type_)
        , method(method_)
    {
    }
    const Type* type;
    const ReflectedStructure::Method* method;

    bool operator<(const SysTypeToMethod& other) const
    {
        const M::SystemProcess* lMeta = method->meta->GetMeta<M::SystemProcess>();
        const M::SystemProcess* rMeta = other.method->meta->GetMeta<M::SystemProcess>();

        return lMeta->group == rMeta->group ? lMeta->order < rMeta->order : lMeta->group < rMeta->group;
    }
};

static const Array<String, 5> groups = { "ENGINE_BEGIN", "GAMEPLAY_BEGIN", "ENGINE_PHYSICS", "GAMEPLAY_END", "ENGINE_END" };

void CollectMethods(UnorderedMap<SP::Type, Set<SysTypeToMethod>>& methods, const Type* type, const Vector<std::unique_ptr<ReflectedStructure::Method>>& systemMethods)
{
    for (const auto& method : systemMethods)
    {
        if (method->meta != nullptr)
        {
            const M::SystemProcess* meta = method->meta->GetMeta<M::SystemProcess>();
            if (meta != nullptr)
            {
                const auto& argsType = method->fn.GetInvokeParams().argsType;
                if (argsType.size() == 2 && argsType[0] == type->Pointer() && (argsType[1]->Is<float32>() || argsType[1]->Is<UIEvent*>()))
                {
                    DVASSERT(methods[meta->type].count({ type, method.get() }) == 0, "Order already in use. System will not be added.");
                    methods[meta->type].emplace(type, method.get());
                }
                else
                {
                    DVASSERT(false, "Method will not be added, arguments are incorrect.");
                }
            }
        }
    }
}
}

void SystemManager::RegisterAllDerivedSceneSystemsRecursively()
{
    //cause can be called several times
    sceneSystems.clear();
    systemsWithoutProcessMethods.clear();
    methodsToProcess.clear();
    methodsToFixedProcess.clear();

    const TypeInheritance* inheritance = Type::Instance<SceneSystem>()->GetInheritance();
    Vector<TypeInheritance::Info> derivedTypes = inheritance->CollectRecursiveDerivedTypes();
    sceneSystems.reserve(derivedTypes.size());

#define PRINT_BY_TAGS 
#ifdef PRINT_BY_TAGS
    UnorderedMap<FastName, Vector<const Type*>> tagToType;
#endif

    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();
    UnorderedMap<SP::Type, Set<SystemManagerDetails::SysTypeToMethod>> methods;

    for (auto& it : derivedTypes)
    {
        const Type* type = it.type;
        const ReflectedType* rType = db->GetByType(type);
        //
        // TODO: Use ReflectionHelpers GetReflectedMeta, when it will be in Engine.
        // Need to move ReflectionHelpers.h from TArc to Engine
        const ReflectedStructure* structure = rType->GetStructure();
        if (structure == nullptr || structure->meta == nullptr)
        {
            Logger::Debug("SystemManager: System '%s' has no meta. System will not be added.", rType->GetPermanentName().c_str());
            continue;
        }

        const M::Tags* tags = structure->meta->GetMeta<M::Tags>();

        if (tags == nullptr)
        {
            Logger::Debug("SystemManager: System '%s' has no tags meta. System will not be added.", rType->GetPermanentName().c_str());
            continue;
        }

        if (rType->GetCtor<Scene*>(type->Pointer()) != nullptr)
        {
#ifdef PRINT_BY_TAGS
            for (FastName tag : tags->tags)
            {
                tagToType[tag].push_back(type);
            }
#endif
            if (!structure->methods.empty())
            {
                sceneSystems.emplace_back();

                SystemManager::SceneTagInfo& tinfo = sceneSystems.back();
                tinfo.systemType = type;
                tinfo.tags = &tags->tags;

                SystemManagerDetails::CollectMethods(methods, type, structure->methods);
            }
            else
            {
                systemsWithoutProcessMethods.emplace_back();

                SystemManager::SceneTagInfo& tinfo = systemsWithoutProcessMethods.back();
                tinfo.systemType = type;
                tinfo.tags = &tags->tags;

                Logger::Debug("SystemManager: system with tags and without methods: '%s'", rType->GetPermanentName().c_str());
            }
        }
        else
        {
            Logger::Debug("SystemManager: There is no (Scene*) constructor for the '%s' system. System will not be added.", rType->GetPermanentName().c_str());
        }
    }

#ifdef PRINT_BY_TAGS
    Logger::Info("System manager: registered systems:");
    for (const auto& p : tagToType)
    {
        Logger::Info("\n%s:", p.first.c_str());
        for (const Type* t : p.second)
        {
            Logger::Info("    %s", db->GetByType(t)->GetPermanentName().c_str());
        }
    }
#endif
    const String format("    %-40s | %-20s : %g");

    int32 currentGroup = -1;

    Logger::Info("\nFixed process methods order:");
    for (const auto& p : methods[SP::Type::FIXED])
    {
        methodsToFixedProcess.emplace_back();

        SystemManager::SceneProcessInfo& pinfo = methodsToFixedProcess.back();
        pinfo.systemType = p.type;
        pinfo.method = &p.method->fn;

        int32 group = static_cast<int32>(p.method->meta->GetMeta<M::SystemProcess>()->group);

        if (currentGroup != group)
        {
            currentGroup = group;
            Logger::Info("\n%s", SystemManagerDetails::groups[currentGroup].c_str());
        }

        Logger::Info(format.c_str(), db->GetByType(p.type)->GetPermanentName().c_str(), p.method->name.c_str(), p.method->meta->GetMeta<M::SystemProcess>()->order);
    }

    currentGroup = -1;

    Logger::Info("\nNormal process methods order:");
    for (const auto& p : methods[SP::Type::NORMAL])
    {
        methodsToProcess.emplace_back();

        SystemManager::SceneProcessInfo& pinfo = methodsToProcess.back();
        pinfo.systemType = p.type;
        pinfo.method = &p.method->fn;

        int32 group = static_cast<int32>(p.method->meta->GetMeta<M::SystemProcess>()->group);

        if (currentGroup != group)
        {
            currentGroup = group;
            Logger::Info("\n%s", SystemManagerDetails::groups[currentGroup].c_str());
        }

        Logger::Info(format.c_str(), db->GetByType(p.type)->GetPermanentName().c_str(), p.method->name.c_str(), p.method->meta->GetMeta<M::SystemProcess>()->order);
    }
}

const Vector<SystemManager::SceneTagInfo>& SystemManager::GetRegisteredSceneSystems() const
{
    return sceneSystems;
}

const Vector<SystemManager::SceneTagInfo>& SystemManager::GetSystemsWithoutProcessMethods() const
{
    return systemsWithoutProcessMethods;
}

const Vector<FastName>& SystemManager::GetTagsForSystem(const Type* systemType)
{
    static Vector<FastName> empty;

    auto it = std::find_if(sceneSystems.begin(), sceneSystems.end(), [systemType](const auto& p) { return p.systemType == systemType; });
    if (it != sceneSystems.end())
    {
        return *it->tags;
    }

    return empty;
}

const Vector<SystemManager::SceneProcessInfo>& SystemManager::GetProcessMethods() const
{
    return methodsToProcess;
}

const Vector<SystemManager::SceneProcessInfo>& SystemManager::GetFixedProcessMethods() const
{
    return methodsToFixedProcess;
}
}
