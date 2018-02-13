#include "REPlatform/Scene/BaseTransformProxies.h"
#include "REPlatform/Scene/SceneHelper.h"
#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/DataNodes/SceneData.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Core/Deprecated.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Particles/ParticleForce.h>
#include <Physics/PhysicsModule.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Particles/ParticleLayer.h>
/*
 * EntityTransformProxy
 */
namespace DAVA
{
const Matrix4& EntityTransformProxy::GetWorldTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    return obj.Cast<Entity>()->GetWorldTransform();
}

Matrix4 EntityTransformProxy::GetLocalTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    return obj.Cast<Entity>()->GetLocalTransform();
}

void EntityTransformProxy::SetLocalTransform(Any& object, const Matrix4& matrix)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    return obj.Cast<Entity>()->SetLocalTransform(matrix);
}

bool EntityTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    using namespace DAVA;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<Entity>());
    Entity* entity = obj.Cast<Entity>();

    if (type == Selectable::TransformType::Scale)
    {
        PhysicsModule* module = GetEngineContext()->moduleManager->GetModule<PhysicsModule>();

        Vector<const Type*> physicsComponents = module->GetBodyComponentTypes();
        const Vector<const Type*>& shapeComponents = module->GetShapeComponentTypes();
        const Vector<const Type*>& vehicleComponents = module->GetVehicleComponentTypes();

        physicsComponents.insert(physicsComponents.end(), shapeComponents.begin(), shapeComponents.end());
        physicsComponents.insert(physicsComponents.end(), vehicleComponents.begin(), vehicleComponents.end());

        for (const Type* type : physicsComponents)
        {
            if (entity->GetComponentCount(type))
            {
                return false;
            }
        }
    }

    return (type == Selectable::TransformType::Disabled) || (entity->GetLocked() == false);
}

bool EntityTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    Selectable dependsOnSelectable(dependsOn);
    if (dependsOnSelectable.CanBeCastedTo<Entity>() == false)
    {
        return false;
    }
    Entity* dependsOnEntity = dependsOnSelectable.Cast<Entity>();
    DVASSERT(dependsOnEntity != nullptr);

    Selectable dependantSelectable(dependant);
    DVASSERT(dependantSelectable.CanBeCastedTo<Entity>());
    Entity* dependantEntity = dependantSelectable.Cast<Entity>();
    DVASSERT(dependantEntity != dependsOnEntity, "[TransformDependsFromObject] One object provided to both parameters");

    return SceneHelper::IsEntityChildRecursive(dependsOnEntity, dependantEntity);
}

/*
 * EmitterTransformProxy
 */
const Matrix4& EmitterTransformProxy::GetWorldTransform(const Any& object)
{
    static Matrix4 currentMatrix;
    currentMatrix.Identity();

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();

    auto ownerComponent = emitterInstance->GetOwner();
    if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        currentMatrix.SetTranslationVector(emitterInstance->GetSpawnPosition());
    }
    else
    {
        const auto& entityTransform = ownerComponent->GetEntity()->GetWorldTransform();
        Vector3 center = emitterInstance->GetSpawnPosition();
        TransformPerserveLength(center, Matrix3(entityTransform));
        currentMatrix.SetTranslationVector(center + entityTransform.GetTranslationVector());
    }
    return currentMatrix;
}

Matrix4 EmitterTransformProxy::GetLocalTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();

    Matrix4 ret;
    ret.SetTranslationVector(emitterInstance->GetSpawnPosition());
    return ret;
}

void EmitterTransformProxy::SetLocalTransform(Any& object, const Matrix4& matrix)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();
    emitterInstance->SetSpawnPosition(Vector3(matrix._30, matrix._31, matrix._32));
}

bool EmitterTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitterInstance = obj.Cast<ParticleEmitterInstance>();
    if (emitterInstance->GetOwner() == nullptr)
        return false;

    return (type == Selectable::TransformType::Disabled) || (type == Selectable::TransformType::Translation);
}

bool EmitterTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    DVASSERT(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    Selectable dependesOnSelectable(dependsOn);
    if (dependesOnSelectable.CanBeCastedTo<Entity>() == false)
    {
        return false;
    }

    Entity* dependsOnEntity = dependesOnSelectable.Cast<Entity>();
    if (dependsOnEntity == nullptr)
        return false;

    Selectable dependantObj(dependant);
    DVASSERT(dependantObj.CanBeCastedTo<ParticleEmitterInstance>());
    ParticleEmitterInstance* emitter = dependantObj.Cast<ParticleEmitterInstance>();
    // check if emitter instance contained inside entity
    ParticleEffectComponent* component = dependsOnEntity->GetComponent<ParticleEffectComponent>();
    if (component != nullptr)
    {
        for (uint32 i = 0, e = component->GetEmittersCount(); i < e; ++i)
        {
            if (component->GetEmitterInstance(i) == emitter)
                return true;
        }
    }

    // or it's children
    for (int32 i = 0, e = dependsOnEntity->GetChildrenCount(); i < e; ++i)
    {
        if (TransformDependsFromObject(dependant, dependsOnEntity->GetChild(i)))
            return true;
    }

    return false;
}

/*
* ForceTransformProxy
*/
const Matrix4& ParticleForceTransformProxy::GetWorldTransform(const Any& object)
{
    static Matrix4 currentMatrix;
    currentMatrix.Identity();

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    ParticleEmitterInstance* emitterInstance = GetEmitterInstance(force);
    if (emitterInstance == nullptr)
        return currentMatrix;

    ParticleEffectComponent* ownerComponent = emitterInstance->GetOwner();
    if ((ownerComponent == nullptr) || (ownerComponent->GetEntity() == nullptr))
    {
        currentMatrix.SetTranslationVector(force->position);
    }
    else
    {
        if (force->worldAlign)
        {
            currentMatrix.SetTranslationVector(force->position + ownerComponent->GetEntity()->GetWorldTransform().GetTranslationVector());
        }
        else
        {
            const Matrix4& entityTransform = ownerComponent->GetEntity()->GetWorldTransform();
            Vector3 center = force->position;
            TransformPerserveLength(center, Matrix3(entityTransform));
            currentMatrix.SetTranslationVector(center + entityTransform.GetTranslationVector());
        }
    }
    return currentMatrix;
}

Matrix4 ParticleForceTransformProxy::GetLocalTransform(const Any& object)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    Matrix4 ret;
    ret.SetTranslationVector(force->position);
    return ret;
}

void ParticleForceTransformProxy::SetLocalTransform(Any& object, const Matrix4& matrix)
{
    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<ParticleForce>());
    ParticleForce* force = obj.Cast<ParticleForce>();

    force->position = matrix.GetTranslationVector();
}

bool ParticleForceTransformProxy::SupportsTransformType(const Any& object, Selectable::TransformType type) const
{
    return (type == Selectable::TransformType::Disabled) || (type == Selectable::TransformType::Translation);
}

bool ParticleForceTransformProxy::TransformDependsFromObject(const Any& dependant, const Any& dependsOn) const
{
    DVASSERT(dependant != dependsOn, "[TransformDependsFromObject] One object provided to both parameters");

    Entity* asEntity = Selectable(dependsOn).AsEntity();
    if (asEntity == nullptr)
        return false;

    Selectable dependantObj(dependant);
    ParticleForce* forceObj = dependantObj.Cast<ParticleForce>();
    // check if force contained inside entity
    ParticleEffectComponent* component = asEntity->GetComponent<ParticleEffectComponent>();
    if (component != nullptr)
    {
        for (uint32 i = 0, e = component->GetEmittersCount(); i < e; ++i)
        {
            ParticleEmitter* emitter = component->GetEmitterInstance(i)->GetEmitter();
            for (ParticleLayer* layer : emitter->layers)
            {
                for (ParticleForce* force : layer->GetParticleForces())
                {
                    if (force == forceObj)
                        return true;
                }
            }
        }
    }

    // or it's children
    for (int32 i = 0, e = asEntity->GetChildrenCount(); i < e; ++i)
    {
        if (TransformDependsFromObject(dependant, asEntity->GetChild(i)))
            return true;
    }

    return true;
}

ParticleEmitterInstance* ParticleForceTransformProxy::GetEmitterInstance(ParticleForce* force) const
{
    DataContext* context = Deprecated::GetActiveContext();
    if (context == nullptr)
        return nullptr;
    EditorParticlesSystem* particleSystem = context->GetData<SceneData>()->GetScene()->GetSystem<EditorParticlesSystem>();
    ParticleLayer* layer = particleSystem->GetForceOwner(force);
    return particleSystem->GetRootEmitterLayerOwner(layer);
}
} // namespace DAVA
