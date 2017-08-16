#include "Scene/System/CollisionSystem/CollisionBox.h"

CollisionBox::CollisionBox(Selectable::Object* object, btCollisionWorld* word, const DAVA::Vector3& position, DAVA::float32 boxSize)
    : CollisionBaseObject(object, word)
{
    Initialize(object, word, position, DAVA::Vector3(boxSize, boxSize, boxSize));
}

CollisionBox::CollisionBox(Selectable::Object* object, btCollisionWorld* word, const DAVA::Vector3& position, const DAVA::Vector3& boxSize)
    : CollisionBaseObject(object, word)
{
    Initialize(object, word, position, boxSize);
}

CollisionBox::~CollisionBox()
{
    if (btObject != nullptr)
    {
        btWord->removeCollisionObject(btObject);
        DAVA::SafeDelete(btObject);
        DAVA::SafeDelete(btShape);
    }
}

void CollisionBox::Initialize(Selectable::Object* object_, btCollisionWorld* world_, const DAVA::Vector3& position_, const DAVA::Vector3& boxSize_)
{
    if (world_ != nullptr)
    {
        DAVA::Vector3 halfSize = 0.5f * boxSize_;

        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(btVector3(position_.x, position_.y, position_.z));

        btShape = new btBoxShape(btVector3(halfSize.x, halfSize.y, halfSize.z));
        btObject = new btCollisionObject();
        btObject->setCollisionShape(btShape);
        btObject->setWorldTransform(trans);
        btWord->addCollisionObject(btObject);

        object.SetBoundingBox(DAVA::AABBox3(-halfSize, halfSize));
    }
}

CollisionBaseObject::ClassifyPlaneResult CollisionBox::ClassifyToPlane(const DAVA::Plane& plane)
{
    return ClassifyBoundingBoxToPlane(object.GetBoundingBox(), TransformPlaneToLocalSpace(plane));
}

CollisionBaseObject::ClassifyPlanesResult CollisionBox::ClassifyToPlanes(const DAVA::Vector<DAVA::Plane>& planes)
{
    for (const DAVA::Plane& plane : planes)
    {
        if (ClassifyToPlane(plane) == CollisionBaseObject::ClassifyPlaneResult::Behind)
        {
            return CollisionBaseObject::ClassifyPlanesResult::Outside;
        }
    }
    return CollisionBaseObject::ClassifyPlanesResult::ContainsOrIntersects;
}