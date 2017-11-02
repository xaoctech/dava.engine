#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodCollObject.h"
#include "REPlatform/Scene/SceneTypes.h"

#include <Render/RenderHelper.h>
#include <Render/UniqueStateSet.h>

namespace DAVA
{
class TextDrawSystem;
struct HoodObject
{
    HoodObject(float32 baseSize);
    virtual ~HoodObject();

    float32 baseSize;
    float32 objScale;
    Color colorX; // axis X
    Color colorY; // axis X
    Color colorZ; // axis X
    Color colorS; // axis selected

    virtual void UpdatePos(const Vector3& pos);
    virtual void UpdateScale(const float32& scale);
    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem) = 0;

    HoodCollObject* CreateLine(const Vector3& from, const Vector3& to);
    Rect DrawAxisText(TextDrawSystem* textDrawSystem, HoodCollObject* x, HoodCollObject* y, HoodCollObject* z);

    Vector<HoodCollObject*> collObjects;

protected:
    Vector3 GetAxisTextPos(HoodCollObject* axis);
};
} // namespace DAVA
