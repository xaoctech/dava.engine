#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

#define ROTATE_HOOD_CIRCLE_PARTS_COUNT 5

namespace DAVA
{
struct RotateHood : public HoodObject
{
    RotateHood();
    ~RotateHood();

    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem);

    HoodCollObject* axisX;
    HoodCollObject* axisY;
    HoodCollObject* axisZ;

    HoodCollObject* axisXc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
    HoodCollObject* axisYc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];
    HoodCollObject* axisZc[ROTATE_HOOD_CIRCLE_PARTS_COUNT];

    float32 modifRotate;

private:
    float32 radius;
};
} // namespace DAVA
