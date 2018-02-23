#include "ENetGuard.h"

#include "Logger/Logger.h"
#include "ENetUtils.h"
#include "enet/enet.h"

namespace DAVA
{
unsigned int ENetGuard::instanceCount = 0;
ENetGuard::ENetGuard()
{
    const bool firstLaunch = instanceCount == 0;
    if (firstLaunch)
    {
        ThrowIfENetError(enet_initialize(), "ENET_INITIALIZE");
    }

    ++instanceCount;
};

ENetGuard::~ENetGuard()
{
    if (0 == --instanceCount)
    {
        enet_deinitialize();
    }
};
} // namespace DAVA
