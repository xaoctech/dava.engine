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
bool IsSpawnNode(DAVA::Entity* entity)
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

void RemoveOldSpawns(RenderSystem* renderSystem, DAVA::UnorderedMap<DAVA::Entity*, DAVA::RenderObject*>& spawnNodes)
{
    using namespace DAVA;

    //remove old spawns: somebody changed type value
    for (auto it = spawnNodes.begin(); it != spawnNodes.end();)
    {
        bool isBot = IsSpawnNode(it->first);
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
    UserNodeSystemDetails::RemoveOldSpawns(renderSystem, spawnNodes);

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
                    auto it = spawnNodes.find(entity);
                    if (it != spawnNodes.end())
                    {
                        RenderObject* ro = it->second;
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
        bool isBot = UserNodeSystemDetails::IsSpawnNode(e);
        if (isBot == true)
        {
            if (spawnNodes.count(e) == 0)
            {
                newSpawns.push_back(e);
            }
        }
    }

    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    for (Entity* e : newSpawns)
    {
        //add RO
        RenderObject* ro = sourceObject->Clone(nullptr);
        ro->SetWorldTransformPtr(UserNodeSystemDetails::GetWorldTransformPtr(e));

        AABBox3 worldBox = editorScene->collisionSystem->GetUntransformedBoundingBox(e);
        DVASSERT(!worldBox.IsEmpty());

        renderSystem->MarkForUpdate(ro);
        renderSystem->RenderPermanent(ro);

        spawnNodes[e] = ro;
    }

    for (auto it = spawnNodes.begin(); it != spawnNodes.end(); ++it)
    {
        if (it->first->GetVisible() && IsSystemEnabled())
        {
            it->second->AddFlag(RenderObject::VISIBLE);
        }
        else
        {
            it->second->RemoveFlag(RenderObject::VISIBLE);
        }
    }
}

void UserNodeSystem::Draw()
{
    using namespace DAVA;

    SceneEditor2* editorScene = static_cast<SceneEditor2*>(GetScene());
    RenderHelper* drawer = editorScene->GetRenderSystem()->GetDebugDrawer();

    for (Entity* entity : userNodes)
    {
        bool isBot = UserNodeSystemDetails::IsSpawnNode(entity);
        //        if ((isBot == false || IsSystemEnabled() == false) && entity->GetVisible())
        {
            AABBox3 worldBox = editorScene->collisionSystem->GetUntransformedBoundingBox(entity);
            DVASSERT(!worldBox.IsEmpty());

            const Matrix4& worldTransform = entity->GetWorldTransform();
            drawer->DrawAABoxTransformed(worldBox, worldTransform, Color(0.5f, 0.5f, 1.0f, 0.3f), RenderHelper::DRAW_SOLID_DEPTH);
            drawer->DrawAABoxTransformed(worldBox, worldTransform, Color(0.2f, 0.2f, 0.8f, 1.0f), RenderHelper::DRAW_WIRE_DEPTH);

            float32 delta = worldBox.GetSize().Length() / 4;
            const Vector3 center = worldTransform.GetTranslationVector();
            const Vector3 xAxis = MultiplyVectorMat3x3(Vector3(delta, 0.f, 0.f), worldTransform);
            const Vector3 yAxis = MultiplyVectorMat3x3(Vector3(0.f, delta, 0.f), worldTransform);
            const Vector3 zAxis = MultiplyVectorMat3x3(Vector3(0.f, 0.f, delta), worldTransform);

            // axises
            drawer->DrawLine(center, center + xAxis, Color(0.7f, 0, 0, 1.0f));
            drawer->DrawLine(center, center + yAxis, Color(0, 0.7f, 0, 1.0f));
            drawer->DrawLine(center, center + zAxis, Color(0, 0, 0.7f, 1.0f));
        }
    }
}
