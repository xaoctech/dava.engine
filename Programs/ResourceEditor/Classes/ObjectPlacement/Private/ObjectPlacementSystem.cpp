#include "Classes/ObjectPlacement/Private/ObjectPlacementSystem.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"
#include "Classes/Selection/Selection.h"

#include <Scene3D/Systems/LandscapeSystem.h>

ObjectPlacementSystem::ObjectPlacementSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(scene);
    modificationSystem = editorScene->modifSystem;
    DVASSERT(modificationSystem != nullptr);
    renderSystem = editorScene->renderSystem;
    DVASSERT(renderSystem != nullptr);
    landscapeSystem = editorScene->landscapeSystem;
    DVASSERT(landscapeSystem != nullptr);
}

bool ObjectPlacementSystem::GetSnapToLandscape() const
{
    DVASSERT(GetScene() != nullptr);
    return snapToLandscape;
}

void ObjectPlacementSystem::SetSnapToLandscape(bool newSnapToLandscape)
{
    DVASSERT(GetScene() != nullptr);
    const DAVA::Vector<DAVA::Entity*>& landscapes = landscapeSystem->GetLandscapeEntities();
    if (landscapes.empty())
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    snapToLandscape = newSnapToLandscape;
    modificationSystem->SetLandscapeSnap(snapToLandscape);
}

void ObjectPlacementSystem::PlaceOnLandscape() const
{
    DVASSERT(GetScene() != nullptr);
    const DAVA::Vector<DAVA::Entity*>& landscapes = landscapeSystem->GetLandscapeEntities();
    if (landscapes.empty())
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    const SelectableGroup& selection = Selection::GetSelection();
    modificationSystem->PlaceOnLandscape(selection);
}

void ObjectPlacementSystem::RemoveEntity(DAVA::Entity* entity)
{
    needCheckLandscapes = true;
}

void ObjectPlacementSystem::Process(DAVA::float32 time)
{
    if (needCheckLandscapes == true)
    {
        const DAVA::Vector<DAVA::Entity*>& landscapes = landscapeSystem->GetLandscapeEntities();
        if (landscapes.empty())
        {
            snapToLandscape = false;
            modificationSystem->SetLandscapeSnap(snapToLandscape);
        }
        needCheckLandscapes = false;
    }
}

void ObjectPlacementSystem::PlaceAndAlign() const
{
    DVASSERT(GetScene() != nullptr);

    using namespace DAVA;

    const SelectableGroup& entities = Selection::GetSelection();

    Vector<EntityToModify> modifEntities = CreateEntityToModifyVector(entities);
    if (modifEntities.empty())
        return;

    for (EntityToModify& etm : modifEntities)
    {
        Matrix4 translation;
        Vector3 collisionNormal;
        bool hitObject = false, hitLandscape = false;

        Vector3 originalPos = etm.originalTransform.GetTranslationVector();
        Ray3 ray(originalPos, Vector3(0.0f, 0.0f, -1.0f));
        RayTraceCollision collision;
        hitObject = renderSystem->GetRenderHierarchy()->RayTrace(ray, collision);
        if (hitObject)
        {
            GetObjectCollisionMatrixAndNormal(collision, translation, collisionNormal);
        }
        else
        {
            Vector3 landscapeCollision;
            DAVA::Vector<DAVA::Landscape*> landscapes = landscapeSystem->GetLandscapeObjects();
            for (Landscape* landscape : landscapes)
            {
                hitLandscape = landscape->PlacePoint(originalPos, landscapeCollision, &collisionNormal);
                if (hitLandscape)
                {
                    translation.SetTranslationVector(landscapeCollision - originalPos);
                }
            }
        }

        if (!(hitLandscape || hitObject))
            continue;

        Vector3 currentUp = Vector3(0.0, 0.0, 1.0);
        Quaternion rotationQuaternion;
        Vector3 pos, scale;
        etm.originalTransform.Decomposition(pos, scale, rotationQuaternion);
        currentUp = currentUp * rotationQuaternion.GetMatrix();
        rotationQuaternion = Quaternion::MakeRotation(currentUp, collisionNormal);
        Matrix4 rotation = rotationQuaternion.GetMatrix();

        etm.object.SetLocalTransform(etm.originalTransform
                                     * etm.toLocalZero * rotation * etm.fromLocalZero
                                     * translation);
    }
    ApplyModificationToScene(GetScene(), modifEntities);
}

void ObjectPlacementSystem::GetObjectCollisionMatrixAndNormal(DAVA::RayTraceCollision& collision,
                                                              DAVA::Matrix4& translation, DAVA::Vector3& normal) const
{
    DAVA::uint16 vertIndices[3];
    collision.geometry->GetTriangleIndices(collision.triangleIndex * 3, vertIndices);
    DAVA::Vector3 v[3];
    collision.geometry->GetCoord(vertIndices[0], v[0]);
    collision.geometry->GetCoord(vertIndices[1], v[1]);
    collision.geometry->GetCoord(vertIndices[2], v[2]);
    normal = (v[1] - v[0]).CrossProduct(v[2] - v[0]);
    translation.SetTranslationVector(DAVA::Vector3(0.0, 0.0, -collision.t));
    DAVA::Matrix4* hitWorldTransform = collision.renderObject->GetWorldTransformPtr();
    DAVA::Quaternion rotationQuaternion;
    DAVA::Vector3 pos, scale;
    hitWorldTransform->Decomposition(pos, scale, rotationQuaternion);
    normal = MultiplyVectorMat3x3(normal, rotationQuaternion.GetMatrix());
}
