#include "Entity/ComponentManager.h"
#include "Entity/Component.h"
#include "Entity/SingletonComponent.h"

#include "Scene3D/Entity.h"
#include "UI/Components/UIComponent.h"
#include "Utils/CRC32.h"
#include "Reflection/ReflectedTypeDB.h"

#include <numeric>

namespace DAVA
{
ComponentManager::ComponentManager()
{
    DVASSERT(runtimeTypeIndex == -1 && componentTypeIndex == -1);

    runtimeTypeIndex = Type::AllocUserData();
    componentTypeIndex = Type::AllocUserData();
}

void ComponentManager::RegisterComponent(const Type* type)
{
    DVASSERT(type != nullptr);

    const Type* sceneComponentType = Type::Instance<Component>();
    const Type* uiComponentType = Type::Instance<UIComponent>();

    if (TypeInheritance::CanDownCast(type, sceneComponentType))
    {
        DVASSERT(!TypeInheritance::CanDownCast(type, uiComponentType));

        auto it = std::find(registeredSceneComponents.begin(), registeredSceneComponents.end(), type);

        if (it != registeredSceneComponents.end())
        {
            DVASSERT(false, "Component is already registered.");
            return;
        }

        ++runtimeSceneComponentsCount;

        type->SetUserData(runtimeTypeIndex, Uint32ToVoidPtr(runtimeSceneComponentsCount));
        type->SetUserData(componentTypeIndex, Uint32ToVoidPtr(ComponentType::SCENE_COMPONENT));

        sceneRuntimeIndexToType.push_back(type);

        DVASSERT(sceneRuntimeIndexToType.size() == runtimeSceneComponentsCount);

        DVASSERT(static_cast<size_t>(runtimeSceneComponentsCount) < ComponentMask().size());

        registeredSceneComponents.push_back(type);
    }
    else if (TypeInheritance::CanDownCast(type, uiComponentType))
    {
        DVASSERT(!TypeInheritance::CanDownCast(type, sceneComponentType));

        auto it = std::find(registeredUIComponents.begin(), registeredUIComponents.end(), type);

        if (it != registeredUIComponents.end())
        {
            DVASSERT(false, "UIComponent is already registered.");
            return;
        }

        ++runtimeUIComponentsCount;

        type->SetUserData(runtimeTypeIndex, Uint32ToVoidPtr(runtimeUIComponentsCount));
        type->SetUserData(componentTypeIndex, Uint32ToVoidPtr(ComponentType::UI_COMPONENT));

        registeredUIComponents.push_back(type);
    }
    else
    {
        DVASSERT(false, "'type' should be derived from Component or UIComponent.");
    }
}

void ComponentManager::PreregisterAllDerivedSceneComponentsRecursively()
{
    DVASSERT(!componentsWerePreregistered);
    DVASSERT(runtimeSceneComponentsCount == 0);
    DVASSERT(registeredSceneComponents.empty() && sceneRuntimeIndexToType.empty());

    const TypeInheritance* inheritance = Type::Instance<Component>()->GetInheritance();

    if (inheritance == nullptr || inheritance->GetDerivedTypes().empty())
    {
        DVASSERT(false, "'Component' type has no derived types.");
        return;
    }

    const Vector<TypeInheritance::Info>& derivedTypes = inheritance->GetDerivedTypes();

    Vector<const Type*> componentsToRegister;
    componentsToRegister.reserve(derivedTypes.size() + derivedTypes.size() / 4); // reserve ~125% of derived types, since each type may have its own children

    for (const TypeInheritance::Info& info : derivedTypes)
    {
        CollectSceneComponentsRecursively(info.type, componentsToRegister);
    }

    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();

    std::sort(componentsToRegister.begin(), componentsToRegister.end(), [db](const Type* l, const Type* r) {
        const String& permanentNameL = db->GetByType(l)->GetPermanentName();
        const String& permanentNameR = db->GetByType(r)->GetPermanentName();

        DVASSERT(!permanentNameL.empty() && !permanentNameR.empty());

        return permanentNameL < permanentNameR;
    });

    CRC32 crc;

    for (const Type* type : componentsToRegister)
    {
        RegisterComponent(type);

        const String& permanentName = db->GetByType(type)->GetPermanentName();
        crc.AddData(permanentName.data(), permanentName.size() * sizeof(String::value_type));
    }

    crc32HashOfPreregisteredComponents = crc.Done();

    componentsWerePreregistered = true;
}

uint32 ComponentManager::GetCRC32HashOfPreregisteredSceneComponents()
{
    DVASSERT(componentsWerePreregistered);

    return crc32HashOfPreregisteredComponents;
}

uint32 ComponentManager::GetCRC32HashOfRegisteredSceneComponents()
{
    UpdateSortedVectors();

    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();

    CRC32 crc;

    for (uint32 x : sceneComponentsSortedByPermanentName)
    {
        const Type* type = registeredSceneComponents[x];
        const String& permanentName = db->GetByType(type)->GetPermanentName();
        crc.AddData(permanentName.data(), permanentName.size());
    }

    return crc.Done();
}

uint32 ComponentManager::GetSortedComponentIndex(const Type* type)
{
    UpdateSortedVectors();

    uint32 runtimeIndex = GetRuntimeComponentIndex(type);

    if (IsRegisteredSceneComponent(type) && runtimeIndex < sceneComponentsSortedByPermanentName.size())
    {
        return sceneComponentsSortedByPermanentName[runtimeIndex];
    }
    else
    {
        DVASSERT(false);
        return 0;
    }
}

const Type* ComponentManager::GetSceneComponentType(uint32 runtimeIndex) const
{
    const Type* ret = nullptr;

    if (runtimeIndex < sceneRuntimeIndexToType.size())
    {
        ret = sceneRuntimeIndexToType[runtimeIndex];
    }

    return ret;
}

void ComponentManager::CollectSceneComponentsRecursively(const Type* type, Vector<const Type*>& components)
{
    components.push_back(type);

    const TypeInheritance* inheritance = type->GetInheritance();

    if (inheritance != nullptr)
    {
        for (const TypeInheritance::Info& info : inheritance->GetDerivedTypes())
        {
            CollectSceneComponentsRecursively(info.type, components);
        }
    }
}

void ComponentManager::UpdateSortedVectors()
{
    const ReflectedTypeDB* db = ReflectedTypeDB::GetLocalDB();

    size_t size = registeredSceneComponents.size();

    if (sceneComponentsSortedByPermanentName.size() != size)
    {
        sceneComponentsSortedByPermanentName.resize(size);
        std::iota(sceneComponentsSortedByPermanentName.begin(), sceneComponentsSortedByPermanentName.end(), 0);

        std::sort(sceneComponentsSortedByPermanentName.begin(), sceneComponentsSortedByPermanentName.end(), [this, db](uint32 l, uint32 r) {
            const ReflectedType* lRefType = db->GetByType(registeredSceneComponents[l]);
            const ReflectedType* rRefType = db->GetByType(registeredSceneComponents[r]);

            DVASSERT(lRefType != nullptr && rRefType != nullptr);

            const String& lName = lRefType->GetPermanentName();
            const String& rName = rRefType->GetPermanentName();

            DVASSERT(!lName.empty() && !rName.empty());

            return lName < rName;
        });
    }
}
}
