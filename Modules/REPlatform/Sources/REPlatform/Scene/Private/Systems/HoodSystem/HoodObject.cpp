#include "REPlatform/Scene/Private/Systems/HoodSystem/HoodObject.h"
#include "REPlatform/Scene/Systems/TextDrawSystem.h"

#include <Render/RenderHelper.h>

namespace DAVA
{
HoodObject::HoodObject(float32 bs)
    : baseSize(bs)
    , objScale(1.0f)
{
}

HoodObject::~HoodObject()
{
    for (size_t i = 0; i < collObjects.size(); i++)
    {
        delete collObjects[i];
    }
}

void HoodObject::UpdatePos(const Vector3& pos)
{
    for (size_t i = 0; i < collObjects.size(); i++)
    {
        collObjects[i]->UpdatePos(pos);
    }
}

void HoodObject::UpdateScale(const float32& scale)
{
    objScale = scale;

    for (size_t i = 0; i < collObjects.size(); i++)
    {
        collObjects[i]->UpdateScale(scale);
    }
}

HoodCollObject* HoodObject::CreateLine(const Vector3& from, const Vector3& to)
{
    HoodCollObject* ret = new HoodCollObject();

    Vector3 direction = (to - from);
    float32 length = direction.Length();
    Vector3 axisX(1, 0, 0);
    Vector3 rotateNormal;
    float32 rotateAngle = 0;
    float32 weight = 0.3f;

    // initially create object on x axis
    ret->btShape = new btCylinderShapeX(btVector3(length / 2, weight / 2, weight / 2));
    ret->btObject = new btCollisionObject();
    ret->btObject->setCollisionShape(ret->btShape);
    ret->baseFrom = from;
    ret->baseTo = to;
    ret->baseOffset = Vector3(length / 2, 0, 0);
    ret->baseRotate.Identity();

    direction.Normalize();
    rotateNormal = axisX.CrossProduct(direction);

    // do we need rotation
    if (!rotateNormal.IsZero())
    {
        rotateNormal.Normalize();
        rotateAngle = acosf(axisX.DotProduct(direction));

        ret->baseRotate.BuildRotation(rotateNormal, -rotateAngle);
    }

    if (0 != rotateAngle)
    {
        btTransform trasf;
        trasf.setIdentity();
        trasf.setRotation(btQuaternion(btVector3(rotateNormal.x, rotateNormal.y, rotateNormal.z), rotateAngle));
        ret->btObject->setWorldTransform(trasf);
    }

    collObjects.push_back(ret);
    return ret;
}

Rect HoodObject::DrawAxisText(TextDrawSystem* textDrawSystem, HoodCollObject* x, HoodCollObject* y, HoodCollObject* z)
{
    Vector2 pos2d;
    float32 maxX = 0, minX = 99999;
    float32 maxY = 0, minY = 99999;

    // x
    pos2d = textDrawSystem->ToPos2d(GetAxisTextPos(x));
    textDrawSystem->DrawText(pos2d, "X", colorX, TextDrawSystem::Align::Center);

    if (pos2d.x > maxX)
        maxX = pos2d.x;
    if (pos2d.x < minX)
        minX = pos2d.x;
    if (pos2d.y > maxY)
        maxY = pos2d.y;
    if (pos2d.y < minY)
        minY = pos2d.y;

    // y
    pos2d = textDrawSystem->ToPos2d(GetAxisTextPos(y));
    textDrawSystem->DrawText(pos2d, "Y", colorY, TextDrawSystem::Align::Center);

    if (pos2d.x > maxX)
        maxX = pos2d.x;
    if (pos2d.x < minX)
        minX = pos2d.x;
    if (pos2d.y > maxY)
        maxY = pos2d.y;
    if (pos2d.y < minY)
        minY = pos2d.y;

    // z
    pos2d = textDrawSystem->ToPos2d(GetAxisTextPos(z));
    textDrawSystem->DrawText(pos2d, "Z", colorZ, TextDrawSystem::Align::Center);

    if (pos2d.x > maxX)
        maxX = pos2d.x;
    if (pos2d.x < minX)
        minX = pos2d.x;
    if (pos2d.y > maxY)
        maxY = pos2d.y;
    if (pos2d.y < minY)
        minY = pos2d.y;

    return Rect(minX, minY, maxX, maxY);
}

Vector3 HoodObject::GetAxisTextPos(HoodCollObject* axis)
{
    Vector3 v = axis->curTo - axis->curFrom;
    return (axis->curTo + v / 8);
}
} // namespace DAVA
