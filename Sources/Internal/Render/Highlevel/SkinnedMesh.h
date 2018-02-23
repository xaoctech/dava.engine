#pragma once

#include "Render/Highlevel/Mesh.h"

namespace DAVA
{
class RenderObject;
class KeyedArchive;
class SerializationContext;
class SkinnedMesh : public Mesh
{
public:
    SkinnedMesh();

    DAVA_DEPRECATED(RenderObject* Clone(RenderObject* newObject) override);
    DAVA_DEPRECATED(void Load(KeyedArchive* archive, SerializationContext* serializationContext) override);
};

} //ns
