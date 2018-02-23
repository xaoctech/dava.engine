#pragma once

#include "Base/BaseTypes.h"
#include "Base/String.h"
#include "Base/FastName.h"

void RegisterGameComponents();

class GameMode
{
public:
    enum Id : DAVA::uint8
    {
        HELLO = 0,
        CARS,
        CHARACTERS,
        PHYSICS,
        TANKS,
        SHOOTER
    };

    static Id IdByName(DAVA::String name);

private:
    static const DAVA::Vector<DAVA::String> idNames;
};

class PlayerKind
{
public:
    enum class Id : DAVA::uint32
    {
        NORMAL_PLAYER,
        RANDOM_BOT,
        SHOOTER_BOT
    };

    PlayerKind(Id id = Id::NORMAL_PLAYER);
    PlayerKind(const DAVA::String& name);
    PlayerKind(DAVA::uint32 id);
    bool IsBot() const;
    DAVA::uint32 AsUInt32() const;
    Id GetId() const;

private:
    Id id;
};

inline PlayerKind::PlayerKind(DAVA::uint32 id_)
    : id(static_cast<Id>(id_))
{
}

inline PlayerKind::PlayerKind(Id id_)
    : id(id_)
{
}

inline PlayerKind::PlayerKind(const DAVA::String& name)
{
    if (name == "random")
    {
        id = Id::RANDOM_BOT;
    }
    else if (name == "shooter")
    {
        id = Id::SHOOTER_BOT;
    }
    else
    {
        id = Id::NORMAL_PLAYER;
    }
}

inline bool PlayerKind::IsBot() const
{
    return (id != Id::NORMAL_PLAYER);
}

inline DAVA::uint32 PlayerKind::AsUInt32() const
{
    return static_cast<DAVA::uint32>(id);
}

inline PlayerKind::Id PlayerKind::GetId() const
{
    return id;
}
