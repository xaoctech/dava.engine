#include "REPlatform/Scene/Private/Systems/HoodSystem/NormalHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

// framework
#include <Render/RenderHelper.h>

namespace DAVA
{
NormalHood::NormalHood()
    : HoodObject(2.0f)
{
    axisX = CreateLine(Vector3(0, 0, 0), Vector3(baseSize, 0, 0));
    axisX->axis = ST_AXIS_X;

    axisY = CreateLine(Vector3(0, 0, 0), Vector3(0, baseSize, 0));
    axisY->axis = ST_AXIS_Y;

    axisZ = CreateLine(Vector3(0, 0, 0), Vector3(0, 0, baseSize));
    axisZ->axis = ST_AXIS_Z;
}

NormalHood::~NormalHood()
{
}

void NormalHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    // x
    drawer->DrawLine(axisX->curFrom, axisX->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    drawer->DrawLine(axisY->curFrom, axisY->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);

    DrawAxisText(textDrawSystem, axisX, axisY, axisZ);
}
} // namespace DAVA
