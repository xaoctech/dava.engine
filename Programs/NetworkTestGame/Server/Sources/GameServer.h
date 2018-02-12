#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>

#include "Utils/StringFormat.h"
#include "Base/FastName.h"
#include "NetworkCore/UDPTransport/UDPServer.h"
#include "Concurrency/Thread.h"

namespace DAVA
{
class Scene;
class NetworkInputSystem;
}

class GameServer
{
public:
    struct Options
    {
        DAVA::Scene* scene;
        DAVA::NetworkInputSystem* inputSystem;
        bool withGUI;
        DAVA::String profilePath;
        bool hasPoisonPill;
    };

    GameServer(DAVA::uint32 host, DAVA::uint16 port, DAVA::uint8 clientsNumber_);
    virtual ~GameServer();

    void Update(DAVA::float32 timeElapsed);
    void Setup(const Options& opts_);

    void SetPoisonPill(DAVA::uint32 framesBeforeKill);

    // SERVER_COMPLETE
    DAVA::FastName arrayOfRandomTokens[128];
    DAVA::uint32 connectedClients = 0;
    const DAVA::FastName& GetNextRandomClientToken()
    {
        arrayOfRandomTokens[connectedClients] = DAVA::FastName(DAVA::Format("%064d", connectedClients).c_str());
        return arrayOfRandomTokens[connectedClients++];
    };

    DAVA::UDPServer& GetUDPServer();

private:
    struct
    {
        bool isActive = false;
        DAVA::uint32 framesBeforeKill = 0;
        DAVA::uint32 framesAfterActive = 0;

        inline bool IsKill() const
        {
            return isActive && framesAfterActive > framesBeforeKill;
        }

    } poisonPill = {};

    DAVA::uint8 clientsNumber;
    DAVA::int64 frameTimeUs = 0;

    DAVA::UDPServer udpServer;

    Options opts;
    void UpdateGUIMode();
    void UpdateConsoleMode();

    DAVA::Thread* dumpThread = nullptr;

    std::mutex lock;
    std::condition_variable notifier;
    bool hasProducedMetrics = false;
};
