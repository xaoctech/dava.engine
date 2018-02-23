#pragma once

#include "Entity/Component.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class VisualScript;

class VisualScriptComponent : public Component
{
protected:
    ~VisualScriptComponent() override;

public:
    VisualScriptComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetScriptFilepath(const FilePath& scriptFilepath);
    const FilePath& GetScriptFilepath() const;

    VisualScript* GetScript();

private:
    FilePath scriptFilepath;
    VisualScript* script = nullptr;

    DAVA_VIRTUAL_REFLECTION(VisualScriptComponent, Component);
};

inline const FilePath& VisualScriptComponent::GetScriptFilepath() const
{
    return scriptFilepath;
}
}
