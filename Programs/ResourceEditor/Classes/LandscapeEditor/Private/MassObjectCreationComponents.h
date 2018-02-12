#pragma once

#include <Entity/Component.h>
#include <FileSystem/KeyedArchive.h>
#include <Scene3D/Entity.h>
#include <Scene3D/SceneFile/SerializationContext.h>

#include <Reflection/Reflection.h>

class MassObjectCreationLayer : public DAVA::Component
{
public:
    Component* Clone(DAVA::Entity* toEntity) override;

    void Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;
    void Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;

    const DAVA::FastName& GetName() const;
    void SetName(const DAVA::FastName& layerName);

private:
    DAVA::FastName layerName = DAVA::FastName("");

    DAVA_VIRTUAL_REFLECTION(MassObjectCreationLayer, DAVA::Component);
};

class MassCreatedObjectComponent : public DAVA::Component
{
public:
    Component* Clone(DAVA::Entity* toEntity) override;

    void Serialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;
    void Deserialize(DAVA::KeyedArchive* archive, DAVA::SerializationContext* serializationContext) override;

    const DAVA::FilePath& GetSourceModelPath() const;
    void SetSourceModelPath(const DAVA::FilePath& path);

private:
    DAVA::FilePath sourceModelPath;
    DAVA_VIRTUAL_REFLECTION(MassCreatedObjectComponent, DAVA::Component);
};