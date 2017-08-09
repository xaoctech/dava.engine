#pragma once

#include "Scene/System/CollisionSystem/CollisionBaseObject.h"

#include <Base/Any.h>

class CollisionBox : public CollisionBaseObject
{
public:
    CollisionBox(const DAVA::Any& object, btCollisionWorld* word, DAVA::Vector3 position, DAVA::float32 boxSize);
    CollisionBox(const DAVA::Any& object, btCollisionWorld* word, const DAVA::Vector3& position, const DAVA::Vector3& boxSize);
    ~CollisionBox();

    CollisionBaseObject::ClassifyPlaneResult ClassifyToPlane(const DAVA::Plane& plane) override;
    CollisionBaseObject::ClassifyPlanesResult ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes) override;

private:
    void Initialize(const DAVA::Any& object, btCollisionWorld* word, const DAVA::Vector3& position, const DAVA::Vector3& boxSize);

private:
    btCollisionShape* btShape = nullptr;
};
