#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
const NetworkID NetworkID::INVALID(~0);
const NetworkID NetworkID::SCENE_ID(0);
const NetworkID NetworkID::FIRST_STATIC_OBJ_ID(static_cast<uint32>(NetworkID::SCENE_ID) + 1);
const NetworkID NetworkID::FIRST_SERVER_ID(0x40000001);
}
