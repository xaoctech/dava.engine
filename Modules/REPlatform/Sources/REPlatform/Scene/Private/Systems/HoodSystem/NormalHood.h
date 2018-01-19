#pragma once

#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"

namespace DAVA
{
struct NormalHood : public HoodObject
{
    NormalHood();
    ~NormalHood();

    virtual void Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem);

    HoodCollObject* axisX;
    HoodCollObject* axisY;
    HoodCollObject* axisZ;
};
} // namespace DAVA
