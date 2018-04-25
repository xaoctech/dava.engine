#include "Entity/SystemManager.h"

#include "Base/FastTags.h"
#include "Logger/Logger.h"
#include "Scene3D/Scene.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
namespace SystemManagerDetails
{
const ReflectedStructure* GetTypeReflectedStructure(const Type& type)
{
    const ReflectedStructure* reflectedStructure = nullptr;

    const ReflectedType* const reflectedType = ReflectedTypeDB::GetLocalDB()->GetByType(&type);

    if (nullptr != reflectedType)
    {
        reflectedStructure = reflectedType->GetStructure();
    }

    return reflectedStructure;
}
} // namespace SystemManagerDetails

bool SystemManager::RegisterSystem(const Type* systemType)
{
    DVASSERT(nullptr != systemType);

    const bool warningsAsAsserts = true;

    const bool isSystemRegistered = RegisterSystem(*systemType, warningsAsAsserts);

    if (isSystemRegistered)
    {
        Logger::Info("\nSystemManager | New system registered: %s", systemType->GetName());

        PrintSystemsSortedProcessMethodsInfo({ systemType });
    }

    return isSystemRegistered;
}

bool SystemManager::IsSystemRegistered(const Type* systemType) const
{
    DVASSERT_ALWAYS(TypeInheritance::CanCast(systemType, Type::Instance<SceneSystem>()));

    return nullptr != systemType && systemsInfo.find(systemType) != systemsInfo.end();
}

const Vector<const Type*>& SystemManager::GetSystems() const
{
    return systems;
}

Vector<const Type*> SystemManager::GetSystems(const FastTags& tags, bool exactMatch) const
{
    Vector<const Type*> matchedSystems;

    for (const auto& p : systemsInfo)
    {
        const Type* systemType = p.first;
        const auto& systemTags = p.second.tags->tags;

        bool needAddSystem = false;

        if (!exactMatch)
        {
            needAddSystem = std::all_of(cbegin(systemTags), cend(systemTags), [& tags = tags.tags](const auto& tag) { return tags.count(tag) != 0; });
        }
        else
        {
            needAddSystem = (systemTags == tags.tags);
        }

        if (needAddSystem)
        {
            matchedSystems.push_back(systemType);
        }
    }

    return matchedSystems;
}

const SystemManager::SystemInfo* SystemManager::GetSystemInfo(const Type* systemType) const
{
    DVASSERT(nullptr != systemType);

    DVASSERT_ALWAYS(TypeInheritance::CanCast(systemType, Type::Instance<SceneSystem>()));

    const SystemInfo* systemInfo = nullptr;

    const auto it = systemsInfo.find(systemType);

    if (it != systemsInfo.cend())
    {
        systemInfo = &it->second;
    }

    DVASSERT(nullptr != systemInfo, Format("System with type '%s' is not registered.", systemType->GetName()).c_str());

    return systemInfo;
}

bool SystemManager::RegisterSystem(const Type& systemType, bool warningsAsAsserts)
{
    auto warning = [warningsAsAsserts](const String& warningMessage)
    {
        !warningsAsAsserts ? Logger::Warning(warningMessage.c_str()) : [&warningMessage]() { DVASSERT(false, warningMessage.c_str()); }();
    };

    DVASSERT_ALWAYS(TypeInheritance::CanCast(&systemType, Type::Instance<SceneSystem>()));

    if (systemsInfo.find(&systemType) != systemsInfo.end())
    {
        warning(Format("System with type '%s' is already registered.", systemType.GetName()));
        return false;
    }

    const ReflectedStructure* const reflectedStructure = SystemManagerDetails::GetTypeReflectedStructure(systemType);

    if (nullptr == reflectedStructure)
    {
        warning(Format("System with type '%s' has no reflected structure, system will not be registered.", systemType.GetName()));
        return false;
    }

    const SystemInfo systemInfo = CollectSystemInfo(*reflectedStructure);

    if (nullptr == systemInfo.tags)
    {
        warning(Format("System with type '%s' has no system tags meta, system will not be registered.", systemType.GetName()));
        return false;
    }

    const bool systemHasRightConstructor = VerifySystemConstructor(reflectedStructure->ctors);

    if (!systemHasRightConstructor)
    {
        warning(Format("System with type '%s' has no constructor with `Scene*` argument, system will not be registered.", systemType.GetName()));
        return false;
    }

    const bool processMethodsVerified = VerifyProcessMethods(systemInfo.processMethods);

    if (!processMethodsVerified)
    {
        DVASSERT_ALWAYS(false, Format("System with type '%s' has incorrect process methods. System will not be registered.", systemType.GetName()).c_str());
        return false;
    }

    systems.push_back(&systemType);
    systemsInfo.emplace(&systemType, std::move(systemInfo));
    systemRegistered.Emit(&systemType, &systemsInfo[&systemType]);

    return true;
}

void SystemManager::PreregisterAllDerivedSceneSystemsRecursively()
{
    const Type* baseSceneSystemType = Type::Instance<SceneSystem>();

    const TypeInheritance* const sceneSystemInheritance = baseSceneSystemType->GetInheritance();

    if (nullptr != sceneSystemInheritance)
    {
        Vector<TypeInheritance::Info> sceneSystemDerivedTypes = sceneSystemInheritance->CollectRecursiveDerivedTypes();

        systems.reserve(sceneSystemDerivedTypes.size());

        for (const TypeInheritance::Info& info : sceneSystemDerivedTypes)
        {
            const Type& systemType = *info.type;

            const bool warningsAsAsserts = false;

            RegisterSystem(systemType, warningsAsAsserts);
        }
    }

    Logger::Info("\nSystemManager | Registered systems:");

    PrintSystemsSortedProcessMethodsInfo(systems);
}

bool SystemManager::VerifySystemConstructor(const Vector<std::unique_ptr<AnyFn>>& systemConstructors) const
{
    for (const std::unique_ptr<AnyFn>& constructor : systemConstructors)
    {
        const Vector<const Type*>& argsTypes = constructor->GetInvokeParams().argsType;

        if (argsTypes.size() == 1 && argsTypes[0]->Is<Scene*>())
        {
            return true;
        }
    }

    return false;
}

bool SystemManager::VerifyProcessMethods(const Vector<SystemProcess>& processMethods) const
{
    for (const SystemProcess& systemProcess : processMethods)
    {
        auto VerifyArgumentsTypes = [](SPI::Type processType, const AnyFn::Params& params) {
            const Vector<const Type*>& argsTypes = params.argsType;
            if (argsTypes.size() == 2 && argsTypes[0]->IsPointer() && TypeInheritance::CanCast(argsTypes[0], Type::Instance<SceneSystem*>()))
            {
                return processType != SPI::Type::Input ? argsTypes[1]->Is<float32>() : argsTypes[1]->Is<UIEvent*>() && params.retType->Is<bool>();
            }
            return false;
        };

        const bool argumentsTypesAreVerified = VerifyArgumentsTypes(systemProcess.info.type, systemProcess.process.GetInvokeParams());

        if (!argumentsTypesAreVerified)
        {
            DVASSERT(false, "Process method has invalid arguments.");
            return false;
        }

        const bool successfulInsert = systemProcessesVerificationCache.insert(systemProcess.info).second;

        if (!successfulInsert)
        {
            DVASSERT(false, "Process method order already in use.");
            return false;
        }
    }

    return true;
}

SystemManager::SystemInfo SystemManager::CollectSystemInfo(const ReflectedStructure& reflectedStructure) const
{
    SystemManager::SystemInfo systemInfo;

    const ReflectedMeta* const reflectedStructureMeta = reflectedStructure.meta.get();

    if (nullptr != reflectedStructureMeta)
    {
        const M::SystemTags* const systemTags = reflectedStructureMeta->GetMeta<M::SystemTags>();

        if (nullptr != systemTags)
        {
            systemInfo.tags = systemTags;
        }
    }

    for (const std::unique_ptr<ReflectedStructure::Method>& method : reflectedStructure.methods)
    {
        const ReflectedMeta* const methodMeta = method->meta.get();

        if (nullptr != methodMeta)
        {
            const M::SystemProcessInfo* const systemProcessInfo = methodMeta->GetMeta<M::SystemProcessInfo>();

            if (nullptr != systemProcessInfo)
            {
                systemInfo.processMethods.emplace_back(method->fn, *systemProcessInfo);
            }
        }
    }

    return systemInfo;
}

void SystemManager::PrintSystemsSortedProcessMethodsInfo(const Vector<const Type*>& systemsTypes) const
{
    struct Info
    {
        Info(const char* systemName, const char* processName, const SystemProcessInfo* processInfo)
            : systemName(systemName)
            , processName(processName)
            , processInfo(processInfo)
        {
        }
        const char* systemName;
        const char* processName;
        const SystemProcessInfo* processInfo;
    };

    Vector<Info> fixedProcesses;
    Vector<Info> processes;
    Vector<Info> inputProcesses;

    for (const Type* systemType : systemsTypes)
    {
        if (!IsSystemRegistered(systemType))
        {
            DVASSERT(false, "System is not registered.");
            continue;
        }

        const ReflectedType* const reflectedType = ReflectedTypeDB::GetLocalDB()->GetByType(systemType);

        if (nullptr == reflectedType) // Should always be true since system is registered, but we don't want to crash in `print` method.
        {
            continue;
        }

        const ReflectedStructure* const reflectedStructure = reflectedType->GetStructure();

        if (nullptr == reflectedStructure) // Should always be true since system is registered, but we don't want to crash in `print` method.
        {
            continue;
        }

        const char* systemName = reflectedType->GetPermanentName().c_str();

        for (const std::unique_ptr<ReflectedStructure::Method>& method : reflectedStructure->methods)
        {
            const ReflectedMeta* const methodMeta = method->meta.get();

            if (nullptr != methodMeta)
            {
                auto insertInfo = [](auto& sortedContainer, const Info& info) {
                    const auto position = std::lower_bound(cbegin(sortedContainer), cend(sortedContainer), info, [](const Info& l, const Info& r) {
                        return *l.processInfo < *r.processInfo;
                    });
                    sortedContainer.insert(position, info);
                };

                const M::SystemProcessInfo* const systemProcessInfo = methodMeta->GetMeta<M::SystemProcessInfo>();

                if (nullptr != systemProcessInfo)
                {
                    Info info(systemName, method->name.c_str(), systemProcessInfo);

                    switch (systemProcessInfo->type)
                    {
                    case SPI::Type::Fixed:
                        insertInfo(fixedProcesses, info);
                        break;
                    case SPI::Type::Normal:
                        insertInfo(processes, info);
                        break;
                    case SPI::Type::Input:
                        insertInfo(inputProcesses, info);
                        break;
                    default:
                        DVASSERT(false, "Unhandled case.");
                        break;
                    }
                }
            }
        }
    }

    const char* format("    %-40s | %-25s : %g");

    const Array<const char*, 3> groups = { "EngineBegin", "Gameplay", "EngineEnd" };

    using T = std::underlying_type_t<SPI::Group>;

    static_assert(static_cast<T>(SPI::Group::EngineBegin) == 0, "");
    static_assert(static_cast<T>(SPI::Group::Gameplay) == 1, "");
    static_assert(static_cast<T>(SPI::Group::EngineEnd) == 2, "");

    int32 currentPrintGroup = -1;

    struct ProcessesTypeNamePair
    {
        ProcessesTypeNamePair(Vector<Info>& processes, const char* name)
            : processes(processes)
            , name(name)
        {
        }
        Vector<Info>& processes;
        const char* name;
    };

    const Array<ProcessesTypeNamePair, 3> processesInfo{ { { fixedProcesses, "Fixed processes:" }, { processes, "Processes:" }, { inputProcesses, "Input processes:" } } };

    for (size_t i = 0; i < 3; ++i)
    {
        if (processesInfo[i].processes.empty())
        {
            continue;
        }

        Logger::Info("\n%s\n", processesInfo[i].name);

        for (const Info& info : processesInfo[i].processes)
        {
            const int32 group = static_cast<int32>(info.processInfo->group);

            if (group != currentPrintGroup)
            {
                Logger::Info(groups[group]);
                currentPrintGroup = group;
            }

            Logger::Info(format, info.systemName, info.processName, info.processInfo->order);
        }

        currentPrintGroup = -1;
    }
}
} // namespace DAVA
