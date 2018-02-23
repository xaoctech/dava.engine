#include "Entity/SystemManager.h"
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"

#include "UI/UISystem.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Logger/Logger.h"

namespace DAVA
{

void SystemManager::PreregisterAllDerivedSceneSystemsRecursively()
{
    const TypeInheritance* inheritance = Type::Instance<SceneSystem>()->GetInheritance();
    Vector<TypeInheritance::Info> derivedTypes = inheritance->CollectRecursiveDerivedTypes();
    sceneSystems.reserve(derivedTypes.size());

    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();
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
            continue;
        }
        const M::Order* meta = structure->meta->GetMeta<M::Order>();
        if (meta)
        {
            sceneSystems.push_back({ it.type, meta->order });
        }
    }

    std::sort(sceneSystems.begin(), sceneSystems.end(),
              [](const std::pair<const Type*, int32>& l, const std::pair<const Type*, int32>& r)
              {
                  return l.second < r.second;
              });

    Logger::Info("[Scene Systems Registered]");
    for (const std::pair<const Type*, int32>& typeWithOrder : sceneSystems)
    {
        const Type* type = typeWithOrder.first;
        int32 order = typeWithOrder.second;
        const String& permanentName = db->GetByType(type)->GetPermanentName();
        Logger::Info("[%s] - [order: %d]", permanentName.c_str(), order);
    }
}

const Vector<std::pair<const Type*, int32>>& SystemManager::GetRegisteredSceneSystems() const
{
    return sceneSystems;
}
}
