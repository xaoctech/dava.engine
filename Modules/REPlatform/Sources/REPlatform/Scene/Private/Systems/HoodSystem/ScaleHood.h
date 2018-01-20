#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

namespace DAVA
{
struct ScaleHood : public HoodObject
{
    ScaleHood();
    ~ScaleHood();

    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem);

    HoodCollObject* axisX;
    HoodCollObject* axisY;
    HoodCollObject* axisZ;

    HoodCollObject* axisXY;
    HoodCollObject* axisXZ;
    HoodCollObject* axisYZ;

    float32 modifScale;
};
} // namespace DAVA
