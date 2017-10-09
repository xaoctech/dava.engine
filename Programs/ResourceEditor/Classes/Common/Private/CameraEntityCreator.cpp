#include "Classes/Common/CameraEntityCreator.h"

#include <TArc/Qt/QtString.h>
#include <TArc/Qt/QtIcon.h>

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

DAVA::Entity* CameraEntityCreator::CreateEntity()
{
    return nullptr;
}

bool CameraEntityCreator::IsComponentsMatched(DAVA::Entity* entity) const
{
    return true;
}

QString CameraEntityCreator::GetEntityTypeName() const
{
    return "";
}

QIcon CameraEntityCreator::GetIcon() const
{
    return QIcon();
}

DAVA_VIRTUAL_REFLECTION_IMPL(CameraEntityCreator)
{
    DAVA::ReflectionRegistrator<CameraEntityCreator>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}