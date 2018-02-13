#include "NetworkPhysics/CharacterMirrorsSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(CharacterMirrorsSingleComponent)
{
    ReflectionRegistrator<CharacterMirrorsSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void CharacterMirrorsSingleComponent::AddMirrorForCharacter(DAVA::Entity* character,
                                                            DAVA::Entity* mirror)
{
    DVASSERT(character != mirror);
    characterToMirror[character] = mirror;
    mirrorToCharacter[mirror] = character;
}

void CharacterMirrorsSingleComponent::RemoveMirrorForCharacter(DAVA::Entity* character)
{
    const auto& it = characterToMirror.find(character);
    if (it != characterToMirror.end())
    {
        mirrorToCharacter.erase(it->second);
        characterToMirror.erase(it);
    }
}

DAVA::Entity* CharacterMirrorsSingleComponent::GetMirrorForCharacter(DAVA::Entity* character) const
{
    const auto& it = characterToMirror.find(character);
    if (it != characterToMirror.end())
    {
        return it->second;
    }

    return nullptr;
}

DAVA::Entity* CharacterMirrorsSingleComponent::GetCharacterFromMirror(DAVA::Entity* mirror) const
{
    const auto& it = mirrorToCharacter.find(mirror);
    if (it != mirrorToCharacter.end())
    {
        return it->second;
    }

    return nullptr;
}

void CharacterMirrorsSingleComponent::Clear()
{
    characterToMirror.clear();
    mirrorToCharacter.clear();
}
