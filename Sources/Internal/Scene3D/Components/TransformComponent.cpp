#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/TransformInterpolationComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TransformComponent)
{
    ReflectionRegistrator<TransformComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::CantBeDeletedManualyComponent(), M::DeveloperModeOnly()]
    .ConstructorByPointer()
    .Field("worldMatrix", &TransformComponent::worldMatrix)[M::ReadOnly(), M::DisplayName("World Transform")]
    .Field("parentMatrix", &TransformComponent::parentMatrix)[M::ReadOnly(), M::HiddenField()]
    .End();
}

Component* TransformComponent::Clone(Entity* toEntity)
{
    TransformComponent* newTransform = new TransformComponent();
    newTransform->SetEntity(toEntity);
    newTransform->position = position;
    newTransform->rotation = rotation;
    newTransform->scale = scale;
    newTransform->worldMatrix = worldMatrix;
    newTransform->parent = this->parent;

    return newTransform;
}

void TransformComponent::SetLocalTransform(const Matrix4& transform)
{
    BeginInterpolation();
    transform.Decomposition(position, scale, rotation);
    ApplyLocalTransfomChanged();
}

void TransformComponent::SetLocalTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
    BeginInterpolation();
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
    ApplyLocalTransfomChanged();
}

void TransformComponent::SetWorldTransform(const Matrix4& transform)
{
    worldMatrix = transform;
    ApplyWorldTransfomChanged();
}

void TransformComponent::SetParent(Entity* parent)
{
    if (nullptr != parent)
    {
        this->parent = parent;
        parentMatrix = &(parent->GetComponent<TransformComponent>()->worldMatrix);
    }
    else
    {
        this->parent = nullptr;
        parentMatrix = nullptr;
    }

    ApplyParentChanged();
}

void TransformComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive)
    {
        archive->SetVector3("tc.translation", position);
        archive->SetQuaternion("tc.rotation", rotation);
        archive->SetVector3("tc.scale", scale);
    }
}

void TransformComponent::Deserialize(KeyedArchive* archive, SerializationContext* sceneFile)
{
    if (NULL != archive)
    {
        if (archive->IsKeyExists("tc.localMatrix"))
        {
            SetLocalTransform(archive->GetMatrix4("tc.localMatrix", Matrix4::IDENTITY));
        }
        else
        {
            SetLocalTransform(archive->GetVector3("tc.translation", position),
                              archive->GetQuaternion("tc.rotation", rotation),
                              archive->GetVector3("tc.scale", scale));
        }
    }

    Component::Deserialize(archive, sceneFile);
}

void TransformComponent::BeginInterpolation()
{
    TransformInterpolationComponent* tic = GetEntity()->GetComponent<TransformInterpolationComponent>();
    if (nullptr != tic)
    {
        tic->Reset();
    }
}

void TransformComponent::ApplyLocalTransfomChanged()
{
    if (!parent)
    {
        worldMatrix = GetLocalTransform();
        ApplyWorldTransfomChanged();
    }

    if (nullptr != entity)
    {
        if (nullptr != entity->GetScene())
        {
            TransformSingleComponent* tsc = entity->GetScene()->GetSingletonComponent<TransformSingleComponent>();
            if (nullptr != tsc)
            {
                tsc->localTransformChanged.push_back(entity);
            }
        }
    }
}

void TransformComponent::ApplyWorldTransfomChanged()
{
    if (nullptr != entity && nullptr != entity->GetScene())
    {
        TransformSingleComponent* tsc = entity->GetScene()->GetSingletonComponent<TransformSingleComponent>();
        if (nullptr != tsc)
        {
            tsc->worldTransformChanged.Push(entity);
        }
    }
}

void TransformComponent::ApplyParentChanged()
{
    if (nullptr != entity && nullptr != entity->GetScene())
    {
        TransformSingleComponent* tsc = entity->GetScene()->GetSingletonComponent<TransformSingleComponent>();
        if (nullptr != tsc)
        {
            tsc->transformParentChanged.push_back(entity);
        }
    }
}
};
