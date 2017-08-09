#pragma once

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"

class CollisionBox : public CollisionBaseObject
{
public:
    CollisionBox(Selectable::Object* object, btCollisionWorld* word, const DAVA::Vector3& position, DAVA::float32 boxSize);
    CollisionBox(Selectable::Object* object, btCollisionWorld* word, const DAVA::Vector3& position, const DAVA::Vector3& boxSize);
    ~CollisionBox();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes) override;

private:
    void Initialize(Selectable::Object* object, btCollisionWorld* word, const DAVA::Vector3& position, const DAVA::Vector3& boxSize);

private:
    btCollisionShape* btShape = nullptr;
};
