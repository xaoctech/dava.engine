#include "Entity/ComponentManager.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "UI/Components/UIComponent.h"
#include "Utils/CRC32.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
#define INT32_TO_VOID_PTR(X) \
reinterpret_cast<void*>(static_cast<intptr_t>(X))

#define VOID_PTR_TO_INT32(X) \
static_cast<int32>(reinterpret_cast<intptr_t>(X))

ComponentManager::ComponentManager()
{
    runtimeTypeIndex = static_cast<int32>(Type::AllocUserData());
    componentType = static_cast<int32>(Type::AllocUserData());
}

void ComponentManager::RegisterComponent(const Type* type)
{
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

        type->SetUserData(runtimeTypeIndex, INT32_TO_VOID_PTR(runtimeSceneComponentsCount + 1));
        type->SetUserData(componentType, INT32_TO_VOID_PTR(eComponentType::SCENE_COMPONENT));

        DVASSERT(sceneRuntimeTypeToType.size() == runtimeSceneComponentsCount);
        sceneRuntimeTypeToType.resize(runtimeSceneComponentsCount + 1);
        sceneRuntimeTypeToType[runtimeSceneComponentsCount] = type;

        ++runtimeSceneComponentsCount;

        DVASSERT(runtimeSceneComponentsCount >= 0 && static_cast<size_t>(runtimeSceneComponentsCount) < ComponentFlags().size());

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

        type->SetUserData(runtimeTypeIndex, INT32_TO_VOID_PTR(runtimeUIComponentsCount + 1));
        type->SetUserData(componentType, INT32_TO_VOID_PTR(eComponentType::UI_COMPONENT));

        ++runtimeUIComponentsCount;

        registeredUIComponents.push_back(type);
    }
    else
    {
        DVASSERT(false);
    }
}

void ComponentManager::RegisterAllDerivedSceneComponentsRecursively()
{
    registeredSceneComponents.clear();

    const Type* type = Type::Instance<Component>();

    const TypeInheritance* inheritance = type->GetInheritance();

    if (inheritance != nullptr)
    {
        for (const TypeInheritance::Info& info : inheritance->GetDerivedTypes())
        {
            RegisterSceneComponentRecursively(info.type);
        }
    }
}

uint32 ComponentManager::GetCRC32HashOfReflectedSceneComponents()
{
    Vector<const String*> componentsPermanentNames;
    componentsPermanentNames.reserve(registeredSceneComponents.size());

    for (const Type* type : registeredSceneComponents)
    {
        const ReflectedType* refType = ReflectedTypeDB::GetLocalDB()->GetByType(type);

        DVASSERT(refType != nullptr);

        const String& permanentName = refType->GetPermanentName();

        if (!permanentName.empty())
        {
            componentsPermanentNames.push_back(&permanentName);
        }
        else
        {
            DVASSERT(false, "Permanent name is not registered for component class.");
        }
    }

    DVASSERT(!componentsPermanentNames.empty(), "CRC32 will be 0.");

    std::sort(componentsPermanentNames.begin(), componentsPermanentNames.end(), [](const String* s1, const String* s2) {
        DVASSERT(s1 != nullptr && s2 != nullptr);
        return *s1 < *s2;
    });

    CRC32 crc;

    for (const String* s : componentsPermanentNames)
    {
        crc.AddData(s->data(), s->size());
    }

    return crc.Done();
}

void ComponentManager::RegisterFakeSceneComponent()
{
    int32 typeIndex = runtimeSceneComponentsCount + 1;

    DVASSERT(sceneRuntimeTypeToType.size() == runtimeSceneComponentsCount);
    sceneRuntimeTypeToType.resize(typeIndex);
    sceneRuntimeTypeToType[typeIndex - 1] = nullptr;

    ++runtimeSceneComponentsCount;

    registeredSceneComponents.push_back(nullptr);
}

const Type* ComponentManager::GetSceneTypeFromRuntimeType(int32 runtimeType) const
{
    const Type* ret = nullptr;
    if (runtimeType >= 0 && runtimeType < static_cast<int32>(sceneRuntimeTypeToType.size()))
    {
        ret = sceneRuntimeTypeToType[runtimeType];
    }

    return ret;
}

void ComponentManager::RegisterSceneComponentRecursively(const Type* type)
{
    RegisterComponent(type);

    const TypeInheritance* inheritance = type->GetInheritance();

    if (inheritance != nullptr)
    {
        for (const TypeInheritance::Info& info : inheritance->GetDerivedTypes())
        {
            RegisterSceneComponentRecursively(info.type);
        }
    }
}
}