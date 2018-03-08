#pragma once

#include <Entity/SingleComponent.h>
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
    DAVA::String gameStatsLogPath;

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

class BattleOptionsSingleComponent : public DAVA::SingleComponent
{
    DAVA_VIRTUAL_REFLECTION(BattleOptionsSingleComponent, DAVA::SingleComponent);

public:
    GameMode::Id gameModeId;

    BattleOptions options;
    BattleControls controls;

    bool isEnemyPredicted;
    bool isEnemyRewound;
    bool compareInputs;
    COLLISION_RESOLVE_MODE collisionResolveMode = COLLISION_RESOLVE_MODE_REWIND_IN_PAST;

    bool isSet = false;
};
