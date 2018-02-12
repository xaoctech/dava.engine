#include "Render/Highlevel/ReflectionProbe.h"

namespace DAVA
{
ReflectionProbe::ReflectionProbe()
{
}

ReflectionProbe::~ReflectionProbe()
{
}

RenderObject* ReflectionProbe::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<ReflectionProbe>(this), "Can clone only Mesh");
        newObject = new ReflectionProbe();
    }

    return RenderObject::Clone(newObject);
}

void ReflectionProbe::SetReflectionType(eType reflectionType_)
{
    reflectionType = reflectionType_;
};

void ReflectionProbe::UpdateProbe()
{
    Matrix4 probeWorldToLocalMatrix = *GetWorldTransformPtr();
    probeWorldToLocalMatrix.Inverse();

    Matrix4 scaleMatrix = Matrix4::MakeScale(2.0f / GetCaptureSize());
    Matrix4 finalWorldToLocal = probeWorldToLocalMatrix * scaleMatrix;

    captureWorldToLocalMatrix = finalWorldToLocal;
    capturePositionInWorldSpace = GetPosition() + GetCapturePosition();
}
};
