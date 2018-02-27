#include "Classes/SplineEditor/Private/SplinePointTransformProxy.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Scene/Systems/SplineEditorSystem.h>

#include <Scene3D/Components/SplineComponent.h>

SplinePointTransformProxy::SplinePointTransformProxy(DAVA::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

const DAVA::Matrix4& SplinePointTransformProxy::GetWorldTransform(const DAVA::Any& object)
{
    using namespace DAVA;

    static Matrix4 currentMatrix;
    static Matrix4 parentMatrix;
    currentMatrix.Identity();

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<SplineComponent::SplinePoint>());
    SplineComponent::SplinePoint* splinePoint = obj.Cast<SplineComponent::SplinePoint>();

    SceneData* sceneData = accessor->GetActiveContext()->GetData<SceneData>();
    SplineEditorSystem* system = sceneData->GetScene()->GetSystem<SplineEditorSystem>();
    SplineComponent* spline = system->GetSplineByPoint(splinePoint);

    if (spline != nullptr)
    {
        parentMatrix = spline->GetEntity()->GetWorldTransform();
        currentMatrix.SetTranslationVector(splinePoint->position);
        currentMatrix = currentMatrix * parentMatrix;
    }

    return currentMatrix;
}

DAVA::Matrix4 SplinePointTransformProxy::GetLocalTransform(const DAVA::Any& object)
{
    return GetWorldTransform(object);
}

void SplinePointTransformProxy::SetLocalTransform(DAVA::Any& object, const DAVA::Matrix4& matrix)
{
    using namespace DAVA;

    Selectable obj(object);
    DVASSERT(obj.CanBeCastedTo<SplineComponent::SplinePoint>());
    SplineComponent::SplinePoint* splinePoint = obj.Cast<SplineComponent::SplinePoint>();

    SceneData* sceneData = accessor->GetActiveContext()->GetData<SceneData>();
    SplineEditorSystem* system = sceneData->GetScene()->GetSystem<SplineEditorSystem>();
    SplineComponent* spline = system->GetSplineByPoint(splinePoint);

    if (spline != nullptr)
    {
        static Matrix4 parentInverseMatrix;
        parentInverseMatrix = spline->GetEntity()->GetWorldTransform();
        parentInverseMatrix.Inverse();
        Matrix4 localMatrix = matrix * parentInverseMatrix;

        splinePoint->position = localMatrix.GetTranslationVector();
    }
}

bool SplinePointTransformProxy::SupportsTransformType(const DAVA::Any& object, DAVA::Selectable::TransformType type) const
{
    return (type == DAVA::Selectable::TransformType::Disabled || type == DAVA::Selectable::TransformType::Translation);
}

bool SplinePointTransformProxy::TransformDependsFromObject(const DAVA::Any& dependant, const DAVA::Any& dependsOn) const
{
    return false;
}