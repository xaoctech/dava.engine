#include <Debug/DVAssert.h>
#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
const NetworkID NetworkID::INVALID = NetworkID(~0);
const NetworkID NetworkID::SCENE_ID = NetworkID(0);

NetworkID NetworkID::CreatePlayerActionId(NetworkPlayerID playerId_7bits, uint32 frameId_20bits, uint32 actionId_4bits)
{
    DVASSERT(playerId_7bits > 0);
    DVASSERT(playerId_7bits < (1 << 7));
    DVASSERT(frameId_20bits < (1 << 20));
    DVASSERT(actionId_4bits < (1 << 4));

    NetworkID ret(0);
    ret.actionValue.id = actionId_4bits;
    ret.actionValue.frameId = frameId_20bits;
    ret.actionValue.playerId = playerId_7bits;
    ret.actionValue.actionFlag = 1;

    return ret;
}

NetworkID NetworkID::CreatePlayerOwnId(NetworkPlayerID playerId_7bits)
{
#ifdef SERVER
    static uint32 counter_23bits = 0;
    counter_23bits++;

    DVASSERT(playerId_7bits >= 0);
    DVASSERT(playerId_7bits < (1 << 7));
    DVASSERT(counter_23bits < (1 << 23));

    NetworkID ret(0);
    ret.playerValue.id = counter_23bits;
    ret.playerValue.playerId = playerId_7bits;
    ret.playerValue.playerFlag = 1;
    ret.playerValue.actionFlag = 0;
    return ret;
#else
    DVASSERT(false && "PlayerOwnId can be created only on server side");
    return INVALID;
#endif
}

NetworkID NetworkID::CreateStaticId(uint32 id_30bits)
{
    DVASSERT(id_30bits > 0);
    DVASSERT(id_30bits < (1 << 29));

    NetworkID ret(0);
    ret.staticValue.id = id_30bits;
    ret.staticValue.playerFlag = 0;
    ret.staticValue.actionFlag = 0;
    return ret;
}

bool NetworkID::IsValid() const
{
    return fullValue != INVALID.fullValue;
}

NetworkPlayerID NetworkID::GetPlayerId() const
{
    DVASSERT(IsValid());

    if (IsPlayerActionId())
    {
        return actionValue.playerId;
    }
    else if (IsPlayerOwnId())
    {
        return playerValue.playerId;
    }

    return 0;
}

uint32 NetworkID::GetPlayerActionFrameId() const
{
    DVASSERT(IsValid());
    DVASSERT(IsPlayerActionId());
    return actionValue.frameId;
}

uint32 NetworkID::GetPlayerActionId() const
{
    DVASSERT(IsValid());
    DVASSERT(IsPlayerActionId());
    return actionValue.id;
}

std::ostream& operator<<(std::ostream& stream, const NetworkID& id)
{
    if (id.IsPlayerActionId())
    {
        stream << "ACT-" << id.actionValue.playerId << "-" << id.actionValue.frameId << "-" << id.actionValue.id;
    }
    else
    {
        if (id.IsPlayerOwnId())
        {
            stream << "OWN-" << id.playerValue.playerId << "-" << id.playerValue.id;
        }
        else
        {
            stream << "STA-" << id.staticValue.id;
        }
    }

    return stream;
}
}
