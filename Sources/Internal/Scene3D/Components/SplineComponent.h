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
    struct SplinePoint
    {
        Vector3 position;
        float32 width;
        float32 value;
    };

    SplineComponent();

    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const Vector<SplinePoint>& GetControlPoints() const;

private:
    Vector<SplinePoint> controlPoints;

public:
    DAVA_VIRTUAL_REFLECTION(SplineComponent, Component);
};
}