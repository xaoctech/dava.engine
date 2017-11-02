#include "REPlatform/Scene/Private/Systems/HoodSystem/MoveHood.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

// framework
#include <Render/RenderHelper.h>

namespace DAVA
{
MoveHood::MoveHood()
    : HoodObject(4.0f)
{
    float32 b = baseSize / 5;
    float32 c = baseSize / 3;

    axisX = CreateLine(Vector3(b, 0, 0), Vector3(baseSize, 0, 0));
    axisX->axis = ST_AXIS_X;

    axisY = CreateLine(Vector3(0, b, 0), Vector3(0, baseSize, 0));
    axisY->axis = ST_AXIS_Y;

    axisZ = CreateLine(Vector3(0, 0, b), Vector3(0, 0, baseSize));
    axisZ->axis = ST_AXIS_Z;

    axisXY1 = CreateLine(Vector3(c, 0, 0), Vector3(c, c, 0));
    axisXY1->axis = ST_AXIS_XY;

    axisXY2 = CreateLine(Vector3(0, c, 0), Vector3(c, c, 0));
    axisXY2->axis = ST_AXIS_XY;

    axisXZ1 = CreateLine(Vector3(c, 0, 0), Vector3(c, 0, c));
    axisXZ1->axis = ST_AXIS_XZ;

    axisXZ2 = CreateLine(Vector3(0, 0, c), Vector3(c, 0, c));
    axisXZ2->axis = ST_AXIS_XZ;

    axisYZ1 = CreateLine(Vector3(0, c, 0), Vector3(0, c, c));
    axisYZ1->axis = ST_AXIS_YZ;

    axisYZ2 = CreateLine(Vector3(0, 0, c), Vector3(0, c, c));
    axisYZ2->axis = ST_AXIS_YZ;
}

MoveHood::~MoveHood()
{
}

void MoveHood::Draw(ST_Axis selectedAxis, ST_Axis mouseOverAxis, RenderHelper* drawer, TextDrawSystem* textDrawSystem)
{
    Color colorSBlend(colorS.r, colorS.g, colorS.b, 0.3f);
    Vector3 curPos = axisX->curPos;

    // arrow length
    float32 arrowLen = axisX->curScale * baseSize / 4;

    // arrow x
    drawer->DrawArrow(axisX->curFrom, axisX->curTo, arrowLen, colorX, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // arrow y
    drawer->DrawArrow(axisY->curFrom, axisY->curTo, arrowLen, colorY, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // arrow z
    drawer->DrawArrow(axisZ->curFrom, axisZ->curTo, arrowLen, colorZ, RenderHelper::DRAW_SOLID_NO_DEPTH);

    // x
    if (selectedAxis & ST_AXIS_X)
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisX->curFrom, axisX->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // y
    if (selectedAxis & ST_AXIS_Y)
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisY->curFrom, axisY->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // z
    if (selectedAxis & ST_AXIS_Z)
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
    else
        drawer->DrawLine(axisZ->curFrom, axisZ->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);

    // xy
    if (selectedAxis == ST_AXIS_XY)
    {
        drawer->DrawLine(axisXY1->curFrom, axisXY1->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXY2->curFrom, axisXY2->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisXY1->curFrom);
        poly.AddPoint(axisXY1->curTo);
        poly.AddPoint(axisXY2->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisXY1->curFrom, axisXY1->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXY2->curFrom, axisXY2->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    // xz
    if (selectedAxis == ST_AXIS_XZ)
    {
        drawer->DrawLine(axisXZ1->curFrom, axisXZ1->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXZ2->curFrom, axisXZ2->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisXZ1->curFrom);
        poly.AddPoint(axisXZ1->curTo);
        poly.AddPoint(axisXZ2->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisXZ1->curFrom, axisXZ1->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisXZ2->curFrom, axisXZ2->curTo, colorX, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    // yz
    if (selectedAxis == ST_AXIS_YZ)
    {
        drawer->DrawLine(axisYZ1->curFrom, axisYZ1->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisYZ2->curFrom, axisYZ2->curTo, colorS, RenderHelper::DRAW_WIRE_NO_DEPTH);

        Polygon3 poly;
        poly.AddPoint(curPos);
        poly.AddPoint(axisYZ1->curFrom);
        poly.AddPoint(axisYZ1->curTo);
        poly.AddPoint(axisYZ2->curFrom);
        drawer->DrawPolygon(poly, colorSBlend, RenderHelper::DRAW_SOLID_NO_DEPTH);
    }
    else
    {
        drawer->DrawLine(axisYZ1->curFrom, axisYZ1->curTo, colorY, RenderHelper::DRAW_WIRE_NO_DEPTH);
        drawer->DrawLine(axisYZ2->curFrom, axisYZ2->curTo, colorZ, RenderHelper::DRAW_WIRE_NO_DEPTH);
    }

    Rect r = DrawAxisText(textDrawSystem, axisX, axisY, axisZ);

    if (!modifOffset.IsZero())
    {
        char tmp[255];
        Vector2 topPos = Vector2((r.x + r.dx) / 2, r.y - 20);

        sprintf(tmp, "[%.2f, %.2f, %.2f]", modifOffset.x, modifOffset.y, modifOffset.z);
        textDrawSystem->DrawText(topPos, tmp, Color(1.0f, 1.0f, 0.0f, 1.0f));
    }
}
} // namespace DAVA
