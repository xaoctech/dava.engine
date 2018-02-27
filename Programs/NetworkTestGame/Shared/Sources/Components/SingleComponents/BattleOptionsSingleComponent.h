#pragma once

#include <Entity/SingletonComponent.h>
#include <Game.h>

namespace DAVA
{
class KeyedArchive;
class UIJoypadComponent;
};

struct BattleOptions
{
    DAVA::FastName token;
    DAVA::String hostName;
    DAVA::uint16 port = 0;
    PlayerKind playerKind;
    bool isDebug = false;
    DAVA::uint32 freqHz = 0;

    static BattleOptions FromKeyedArchive(DAVA::KeyedArchive* archive);
};

struct BattleControls
{
    DAVA::UIControl* finalAim = nullptr;
    DAVA::UIControl* currentAim = nullptr;
    DAVA::UIControl* interactControl = nullptr;
    DAVA::UIJoypadComponent* movementJoypad = nullptr;
};

enum COLLISION_RESOLVE_MODE
{
    COLLISION_RESOLVE_MODE_NONE,
    COLLISION_RESOLVE_MODE_SERVER_COLLISIONS,
    COLLISION_RESOLVE_MODE_REWIND_IN_PAST
};

class BattleOptionsSingleComponent : public DAVA::SingletonComponent
{
    DAVA_VIRTUAL_REFLECTION(BattleOptionsSingleComponent, DAVA::SingletonComponent);

public:
    void Clear() override;

    BattleOptions options;
    BattleControls controls;

    GameMode::Id gameModeId;
    bool isEnemyPredicted;
    DAVA::String gameStatsLogPath;
    bool compareInputs;
    COLLISION_RESOLVE_MODE collisionResolveMode = COLLISION_RESOLVE_MODE_REWIND_IN_PAST;

    bool isSet = false;
};
