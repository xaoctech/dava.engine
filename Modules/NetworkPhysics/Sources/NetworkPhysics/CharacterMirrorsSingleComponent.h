#pragma once

#include <Base/UnordererMap.h>
#include <Entity/SingleComponent.h>

namespace DAVA
{
class Entity;
}

class CharacterMirrorsSingleComponent : public DAVA::SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(CharacterMirrorsSingleComponent, DAVA::SingleComponent);

public:
    void AddMirrorForCharacter(DAVA::Entity* character, DAVA::Entity* mirror);
    void RemoveMirrorForCharacter(DAVA::Entity* character);
    DAVA::Entity* GetMirrorForCharacter(DAVA::Entity* character) const;
    DAVA::Entity* GetCharacterFromMirror(DAVA::Entity* entity) const;
    const DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*>& GetCharacterToMirrorMap() const;

private:
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*> characterToMirror;
    DAVA::UnorderedMap<DAVA::Entity*, DAVA::Entity*> mirrorToCharacter;
};
