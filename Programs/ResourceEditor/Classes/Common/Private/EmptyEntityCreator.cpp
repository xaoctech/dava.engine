#include "Classes/Common/EmptyEntityCreator.h"

#include <TArc/Utils/Utils.h>

#include <Scene3D/Entity.h>
#include <Reflection/ReflectionRegistrator.h>

DAVA::Entity* EmptyEntityCreator::CreateEntity()
{
    return new DAVA::Entity();
}

bool EmptyEntityCreator::IsComponentsMatched(DAVA::Entity* entity) const
{
    return true;
}

QString EmptyEntityCreator::GetEntityTypeName() const
{
    return QStringLiteral("Empty Entity");
}

QIcon EmptyEntityCreator::GetIcon() const
{
    return DAVA::TArc::SharedIcon(":/QtIcons/node.png");
}

DAVA_VIRTUAL_REFLECTION_IMPL(EmptyEntityCreator)
{
    DAVA::ReflectionRegistrator<EmptyEntityCreator>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer()
    .End();
}
