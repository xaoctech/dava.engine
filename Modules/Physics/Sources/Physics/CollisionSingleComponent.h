#pragma once

#include <Base/Vector.h>
#include <Entity/SingletonComponent.h>
#include <Math/Vector.h>

namespace DAVA
{
class Entity;

/** Specifies information about collision point */
struct CollisionPoint
{
    /** Collision point coordinates */
    Vector3 position;

    /** Collision point normal from second body to the first */
    Vector3 normal;

    /** Impulse applied at the point */
    Vector3 impulse;
};

/** Specifies information about collision */
struct CollisionInfo
{
    /** First collision object */
    Entity* first = nullptr;

    /** Second collision object */
    Entity* second = nullptr;

    /** Vector of collision points */
    Vector<CollisionPoint> points;
};

/** Specifies information about trigger event */
struct TriggerInfo
{
    /** Trigger entity */
    Entity* trigger;
    /** Captured entity */
    Entity* other;
};

/** Single component providing information about current collisions */
class CollisionSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(CollisionSingleComponent, SingletonComponent);

    /** Vector of currently active triggers */
    Vector<TriggerInfo> activeTriggers;
    /** Vector of current collisions */
    Vector<CollisionInfo> collisions;

    void GetEntitesByTrigger(Entity* trigger, UnorderedSet<Entity*>& result) const;
    void GetTriggersByEntity(Entity* entity, UnorderedSet<Entity*>& result) const;

    /** Remove collision from current collisions vector with specified `entity` */
    void RemoveCollisionsWithEntity(Entity* entity);

    Vector<CollisionInfo> GetCollisionsWithEntity(Entity* entity) const;
    void GetCollisionEntities(Entity* entity, UnorderedSet<Entity*>& result) const;
};
}
