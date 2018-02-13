#pragma once

#include <Base/UnordererMap.h>
#include <Entity/SingletonComponent.h>

namespace DAVA
{
class Entity;
}

class CharacterMirrorsSingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(CharacterMirrorsSingleComponent, DAVA::SingletonComponent);

public:
    void AddMirrorForCharacter(DAVA::Entity* character, DAVA::Entity* mirror);
    void RemoveMirrorForCharacter(DAVA::Entity* character);
    DAVA::Entity* GetMirrorForCharacter(DAVA::Entity* character) const;
    DAVA::Entity* GetCharacterFromMirror(DAVA::Entity* entity) const;
    void Clear() override;

private:
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*> characterToMirror;
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*> mirrorToCharacter;
};
