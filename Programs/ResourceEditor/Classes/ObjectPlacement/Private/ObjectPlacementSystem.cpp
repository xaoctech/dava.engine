#include "Classes/ObjectPlacement/Private/ObjectPlacementSystem.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"

ObjectPlacementSystem::ObjectPlacementSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(scene);
    modificationSystem = editorScene->modifSystem;
    renderSystem = editorScene->renderSystem;
}

bool ObjectPlacementSystem::GetSnapToLandscape() const
{
    return snapToLandscape;
}

void ObjectPlacementSystem::SetSnapToLandscape(bool newSnapToLandscape)
{
    if (!GetScene())
        return;

    if (nullptr == landscape)
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    snapToLandscape = newSnapToLandscape;
    DVASSERT(modificationSystem);
    modificationSystem->SetLandscapeSnap(snapToLandscape);
}

void ObjectPlacementSystem::PlaceOnLandscape() const
{
    if (!GetScene())
        return;

    if (nullptr == landscape)
    {
        DAVA::Logger::Error(ResourceEditor::NO_LANDSCAPE_ERROR_MESSAGE.c_str());
        return;
    }
    DVASSERT(modificationSystem);
    const SelectableGroup& selection = Selection::GetSelection();
    modificationSystem->PlaceOnLandscape(selection);
}

void ObjectPlacementSystem::AddEntity(DAVA::Entity* entity)
{
    if (GetLandscape(entity) != NULL)
    {
        landscape = GetLandscape(entity);
    }
}

void ObjectPlacementSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (GetLandscape(entity) != NULL)
    {
        snapToLandscape = false;
        landscape = nullptr;
        DVASSERT(modificationSystem);
        modificationSystem->SetLandscapeSnap(snapToLandscape);
    }
}

void ObjectPlacementSystem::PlaceAndAlign() const
{
    using namespace DAVA;

    const SelectableGroup& entities = Selection::GetSelection();

    Vector<EntityToModify> modifEntities;
    modifEntities = CreateEntityToModifyVector(entities);
    if (modifEntities.empty())
        return;

    for (EntityToModify& etm : modifEntities)
    {
        Matrix4 translation;
        Vector3 collisionNormal;
        bool hitObject = false, hitLandscape = false;
        do
        {
            Vector3 originalPos = etm.originalTransform.GetTranslationVector();
            Ray3 ray(originalPos, Vector3(0.0f, 0.0f, -1.0f));
            RayTraceCollision collision;
            hitObject = renderSystem->GetRenderHierarchy()->RayTrace(ray, collision);
            if (hitObject)
            {
                GetObjectCollisionMatrixAndNormal(collision, translation, collisionNormal);
                break;
            }

            Vector3 landscapeCollision;
            if (nullptr != landscape)
            {
                hitLandscape = landscape->PlacePoint(originalPos, landscapeCollision, &collisionNormal);
            }
            if (hitLandscape)
            {
                translation.SetTranslationVector(landscapeCollision - originalPos);
            }
        } while (0);

        if (! (hitLandscape || hitObject))
            continue;

        Vector3 currentUp = Vector3(0.0, 0.0, 1.0);
        Quaternion rotationQuaternion;
        Vector3 _pos, _scale;
        etm.originalTransform.Decomposition(_pos, _scale, rotationQuaternion);
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
}
