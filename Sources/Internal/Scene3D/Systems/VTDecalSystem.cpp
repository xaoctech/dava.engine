#include "Scene3D/Systems/VTDecalSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"

namespace DAVA
{
VTDecalSystem::VTDecalSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<VTDecalComponent>())
{
}

void VTDecalSystem::Process(float32 timeElapsed)
{
    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();
    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<VTDecalComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();

                // Update new transform pointer, and mark that transform is changed
                TransformComponent* transformComponent = GetTransformComponent(entity);
                const Matrix4* worldTransformPointer = transformComponent->GetWorldTransformPtr();

                RenderObject* object = decalComponent->renderObject;
                if (NULL != object)
                {
                    object->SetWorldTransformPtr(worldTransformPointer);
                    entity->GetScene()->renderSystem->MarkForUpdate(object);
                }
            }
        }
    }
    VTSingleComponent* vtc = GetScene()->GetSingletonComponent<VTSingleComponent>();
    if (vtc)
    {
        for (Entity* entity : vtc->vtDecalChanged)
        {
            VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();
            decalComponent->renderObject->SetDecalSize(decalComponent->decalSize);
            decalComponent->renderObject->SetMaterial(decalComponent->material.Get());
            decalComponent->renderObject->SetSortingOffset(decalComponent->sortingOffset);
            GetScene()->renderSystem->MarkForUpdate(decalComponent->renderObject);
        }
        for (Entity* entity : vtc->vtSplineChanged) //we need to recompute bbox and update it in vt hierarchy
        {
            VTDecalComponent* decalComponent = entity->GetComponent<VTDecalComponent>();
            if (decalComponent)
                GetScene()->renderSystem->MarkForUpdate(decalComponent->renderObject);
        }
    }
}

void VTDecalSystem::NotifyMaterialChanged(NMaterial* material)
{
    for (DecalRenderObject* decal : decalObjects)
        if (decal->GetMaterial() == material)
            GetScene()->renderSystem->MarkForUpdate(decal);
}

void VTDecalSystem::AddEntity(Entity* entity)
{
    VTDecalComponent* component = entity->GetComponent<VTDecalComponent>();
    if (component)
    {
        ScopedPtr<DecalRenderObject> renderObject(new DecalRenderObject());
        renderObject->SetDomain(DecalRenderObject::DOMAIN_VT);
        TransformComponent* transformComponent = GetTransformComponent(entity);
        const Matrix4* worldTransformPointer = transformComponent->GetWorldTransformPtr();
        renderObject->SetWorldTransformPtr(worldTransformPointer);
        renderObject->SetMaterial(component->GetMaterial());
        renderObject->SetDecalSize(component->GetLocalSize());
        renderObject->SetSortingOffset(component->GetSortingOffset());
        component->SetRenderObject(renderObject);
        GetScene()->GetRenderSystem()->RenderPermanent(renderObject);
        decalObjects.push_back(renderObject);
    }
}
void VTDecalSystem::RemoveEntity(Entity* entity)
{
    VTDecalComponent* component = entity->GetComponent<VTDecalComponent>();
    if (component && component->GetRenderObject())
    {
        DecalRenderObject* decalObject = component->GetRenderObject();

        GetScene()->GetRenderSystem()->RemoveFromRender(decalObject);
        FindAndRemoveExchangingWithLast(decalObjects, decalObject);
    }
}

void VTDecalSystem::PrepareForRemove()
{
    for (DecalRenderObject* object : decalObjects)
    {
        GetScene()->GetRenderSystem()->RemoveFromRender(object);
    }
    decalObjects.clear();
}
}
