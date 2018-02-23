#pragma once

#include <Reflection/Reflection.h>

#include <NetworkCore/UDPTransport/UDPClient.h>

namespace DAVA
{
class Scene;
}

class GameClient
{
    DAVA_REFLECTION(GameClient);

public:
    struct Options
    {
        DAVA::Scene* scene = nullptr;
        bool isBot;
    };

    GameClient(const DAVA::String& hostName, DAVA::uint16 port, const DAVA::FastName& token);
    void Update(DAVA::float32 timeElapsed);
    void Setup(const Options& opts);

    void OnConnect();
    void OnDisconnect();

    GameClient::Options* GetOptions();
    DAVA::UDPClient& GetUDPClient();
    DAVA::UDPClient* GetUDPClientPtr();

private:
    enum class State
    {
        Idle,
        Connected,
        Established
    };

    struct
    {
        State state = State::Idle;
        DAVA::uint32 entityId = 0;
    } serviceInfo;

    DAVA::int64 frameTimeUs = 0;

    DAVA::UDPClient udpClient;
    Options opts;

    void ReceiveSyncPacket(const DAVA::uint8* data, size_t size);
};

inline DAVA::UDPClient* GameClient::GetUDPClientPtr()
{
    return &udpClient;
}

inline GameClient::Options* GameClient::GetOptions()
{
    return &opts;
};
