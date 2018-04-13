#pragma once

#include "TArc/Core/ClientModule.h"
#include "TArc/Core/FieldBinder.h"
#include "TArc/Utils/QtConnections.h"
#include "Base/Any.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class RenderObject;
class Entity;
}

class ImpostersModule : public DAVA::ClientModule
{
protected:
    void PostInit() override;

private:
    void Bake();
    void BakeRenderObject(DAVA::Entity* entity, DAVA::RenderObject* object, const DAVA::String& textureFileName);

private:
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;
    DAVA::QtConnections connections;

    DAVA_VIRTUAL_REFLECTION(ReflectionsModule, DAVA::ClientModule);
};
