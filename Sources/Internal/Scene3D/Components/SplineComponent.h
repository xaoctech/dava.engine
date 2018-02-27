#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class SplineComponent : public Component
{
public:
    //GFX_COMPLETE - different metadata for spline? at least tangent?
    //GFX_COMPLETE - dte uses 2d spline data as it is going to be in VT anyway, may be we should as well - anyway objects are gonna be placed on landscape (whats with bridges?)
    struct SplinePoint : public ReflectionBase
    {
        Vector3 position;
        float32 width = 1.f;
        float32 value = 1.f;
        DAVA_VIRTUAL_REFLECTION(SplinePoint, ReflectionBase);
    };
    using SplinePoints = Vector<SplinePoint*>;

    SplineComponent();
    ~SplineComponent();

    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const SplinePoints& GetControlPoints() const;

    SplinePoints controlPoints;

    DAVA_VIRTUAL_REFLECTION(SplineComponent, Component);
};
}