#include "Classes/UserNodeModule/Private/UserNodeSystem.h"

#include <FileSystem/KeyedArchive.h>
#include <Debug/DVAssert.h>
#include <Render/Highlevel/RenderObject.h>
#include <Render/Highlevel/RenderSystem.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Scene.h>
#include <Utils/Utils.h>

namespace UserNodeSystemDetails
{
bool IsSpawnNote(DAVA::Entity* entity)
{
    using namespace DAVA;

    KeyedArchive* options = GetCustomPropertiesArchieve(entity);
    if (options != nullptr && options->IsKeyExists("type"))
    {
        String type = options->GetString("type");
        return type == "spawnpoint" || (type == "botspawn");
    }

    return false;
}

DAVA::Matrix4* GetWorldTransformPtr(DAVA::Entity* entity)
{
    using namespace DAVA;
    return (static_cast<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT)))->GetWorldTransformPtr();
}
}

UserNodeSystem::UserNodeSystem(DAVA::Scene* scene, DAVA::RenderObject* object)
    : SceneSystem(scene)
    , sourceObject(DAVA::SafeRetain(object))
{
}

UserNodeSystem::~UserNodeSystem()
{
    SafeRelease(sourceObject);

    DVASSERT(userNodes.empty());
    DVASSERT(spawnNodes.empty());
}

void UserNodeSystem::AddEntity(DAVA::Entity* entity)
{
    userNodes.push_back(entity);
}

void UserNodeSystem::RemoveEntity(DAVA::Entity* entity)
{
    auto it = spawnNodes.find(entity);
    if (it != spawnNodes.end())
    {
        GetScene()->GetRenderSystem()->RemoveFromRender(it->second);
        SafeRelease(it->second);
        spawnNodes.erase(it);
    }

    DAVA::FindAndRemoveExchangingWithLast(userNodes, entity);
}

void UserNodeSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    if (userNodes.empty() == true || sourceObject == nullptr)
        return;

    RenderSystem* renderSystem = GetScene()->GetRenderSystem();

    //remove old spawns: somebody changed type value
    for (auto it = spawnNodes.begin(); it != spawnNodes.end();)
    {
        bool isBot = UserNodeSystemDetails::IsSpawnNote(it->first);
        if (isBot == false)
        {
            //Remove and deregister RO
            renderSystem->RemoveFromRender(it->second);
            SafeRelease(it->second);

            it = spawnNodes.erase(it);
        }
        else
        {
            ++it;
        }
    }

    //update transform of entity
    TransformSingleComponent* trSingle = GetScene()->transformSingleComponent;
    if (trSingle != nullptr)
    {
        for (auto& pair : trSingle->worldTransformChanged.map)
        {
            if (pair.first->GetComponentsCount(Component::USER_COMPONENT) > 0)
            {
                for (Entity* entity : pair.second)
                {
                    if (spawnNodes.count(entity) > 0)
                    {
                        RenderObject* ro = spawnNodes.at(entity);
                        ro->SetWorldTransformPtr(UserNodeSystemDetails::GetWorldTransformPtr(entity));
                        renderSystem->MarkForUpdate(ro);
                    }
                }
            }
        }
    }

    //find new spawns: somebody changed type
    Vector<Entity*> newSpawns;
    for (Entity* e : userNodes)
    {
        bool isBot = UserNodeSystemDetails::IsSpawnNote(e);
        if (isBot == true)
        {
            if (spawnNodes.count(e) == 0)
            {
                newSpawns.push_back(e);
            }
        }
    }
    for (Entity* e : newSpawns)
    {
        //add RO
        RenderObject* ro = sourceObject->Clone(nullptr);
        ro->SetWorldTransformPtr(UserNodeSystemDetails::GetWorldTransformPtr(e));
        renderSystem->MarkForUpdate(ro);
        renderSystem->RenderPermanent(ro);

        spawnNodes[e] = ro;
    }
}
