#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include "NetworkCore/UDPTransport/UDPServer.h"
#include "NetworkCore/UDPTransport/UDPClient.h"

using namespace DAVA;

const int PORT = 8841;

class IServerClientConsumer :
public DAVA::UnitTests::TestClass
{
public:
    void OnConnect(const Responder& responder){};
    void OnConnect(){};
    void OnDisconnect(){};
    void OnError(NetworkErrors error){};
};

class GameClient : public UDPClient
{
public:
    GameClient()
        : UDPClient("127.0.0.1", PORT, GenerateToken(), 1)
        , idx(gIDx)
    {
        SubscribeOnConnect(OnClientConnectCb(this, &GameClient::OnConnect));
        SubscribeOnDisconnect(OnClientDisconnectCb(this, &GameClient::OnDisconnect));
        SubscribeOnError(OnClientErrorCb(this, &GameClient::OnError));
        SubscribeOnReceive(PacketParams::DEFAULT_CHANNEL_ID, OnClientReceiveCb(this, &GameClient::OnReceive));
        ++gIDx;
    }

    static FastName GenerateToken()
    {
        return FastName(Format("%064d", gIDx));
    }

    void Send(const uint8* data, size_t size, const PacketParams& param)
    {
        UDPClient::Send(data, size, param);
        if (param.channelID != PacketParams::TOKEN_CHANNEL_ID)
        {
            ++sndCount[idx];
        }
    }

    void OnReceive(const uint8* data, size_t size, uint8, uint32)
    {
        ++rcvCount[idx];
    }

    void SetCounters(int* snd, int* rcv)
    {
        sndCount = snd;
        rcvCount = rcv;
    }

    void OnError(NetworkErrors error)
    {
    }

    void OnConnect()
    {
    }

    void OnDisconnect()
    {
    }

    const int idx;

private:
    static int gIDx;
    int* sndCount;
    int* rcvCount;
};

int GameClient::gIDx = 0;

DAVA_TESTCLASS_CUSTOM_BASE(UDPTransportTest, IServerClientConsumer)
{
    const int MAX_PACKET_COUNT = 10;
    const int MAX_EPOCH_COUNT = 3;
    const int MAX_ITER_COUNT = 10000;
    const PacketParams DEF_PACKET_PARAMS = {};

    int sndCount_ = 0;
    int rsdCount_ = 0;
    int rcvCount_ = 0;

    void OnServerReceive(const Responder& responder, const uint8* data, size_t size)
    {
        ++rsdCount_;
        responder.Send(data, size, DEF_PACKET_PARAMS);
    }

    void OnClientReceive(const uint8* data, size_t, uint8, uint32)
    {
        const int* packetID = reinterpret_cast<const int*>(data);
        TEST_VERIFY(*packetID == ++rcvCount_);
    }

    DAVA_TEST (OneServerOneClient)
    {
        for (int i = 0; i < MAX_EPOCH_COUNT; i++)
        {
            sndCount_ = 0;
            rsdCount_ = 0;
            rcvCount_ = 0;

            UDPServer udpServer(ENET_HOST_ANY, PORT, 1);
            udpServer.SubscribeOnReceive(PacketParams::DEFAULT_CHANNEL_ID,
                                         OnServerReceiveCb(this, &UDPTransportTest::OnServerReceive));
            FastName token("0000000000000000000000000000000000000000000000000000000000000001");
            UDPClient udpClient("127.0.0.1", PORT, token, 1);
            udpClient.SubscribeOnReceive(PacketParams::DEFAULT_CHANNEL_ID,
                                         OnClientReceiveCb(this, &UDPTransportTest::OnClientReceive));
            int iterCount = 0;
            while (!udpClient.IsConnected())
            {
                TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
                udpClient.Update();
                udpServer.Update();
            }

            TokenPacketHeader header;
            uint32 tokenLength = static_cast<uint32>(strlen(token.c_str()));
            DVASSERT(tokenLength == TokenPacketHeader::TOKEN_LENGTH);
            memcpy(header.token, token.c_str(), tokenLength);
            udpClient.Send((uint8*)&header, sizeof(TokenPacketHeader), PacketParams::Reliable(PacketParams::TOKEN_CHANNEL_ID));
            iterCount = 0;
            while (!udpServer.HasResponder(token))
            {
                TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
                udpClient.Update();
                udpServer.Update();
            }
            udpServer.SetValidToken(token);

            iterCount = 0;
            while (rcvCount_ < MAX_PACKET_COUNT)
            {
                TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
                if (rcvCount_ == sndCount_)
                {
                    ++sndCount_;
                    udpClient.Send((uint8*)&sndCount_, sizeof(sndCount_), DEF_PACKET_PARAMS);
                }

                udpClient.Update();
                udpServer.Update();
            }

            TEST_VERIFY(sndCount_ == MAX_PACKET_COUNT);
            TEST_VERIFY(rsdCount_ == MAX_PACKET_COUNT);
            TEST_VERIFY(rcvCount_ == MAX_PACKET_COUNT);
        }
    };

    DAVA_TEST (OneServerManyClient)
    {
        const int NUM_CLIENTS = 3;
        rsdCount_ = 0;
        int sndCount[NUM_CLIENTS] = {};
        int rcvCount[NUM_CLIENTS] = {};
        int rsdCount[NUM_CLIENTS] = {};

        UDPServer udpServer(ENET_HOST_ANY, PORT, NUM_CLIENTS);
        udpServer.SubscribeOnReceive(PacketParams::DEFAULT_CHANNEL_ID,
                                     OnServerReceiveCb(this, &UDPTransportTest::OnServerReceive));
        GameClient gameClients[NUM_CLIENTS];
        int iterCount = 0;
        while (true)
        {
            TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
            bool allConnected = true;
            for (auto& gameClient : gameClients)
            {
                gameClient.SetCounters(sndCount, rcvCount);
                gameClient.Update();
                allConnected &= gameClient.IsConnected();
            }
            if (allConnected)
            {
                break;
            }
            udpServer.Update();
        }

        for (auto& gameClient : gameClients)
        {
            TokenPacketHeader header;
            uint32 tokenLength = static_cast<uint32>(strlen(gameClient.GetAuthToken().c_str()));
            DVASSERT(tokenLength == TokenPacketHeader::TOKEN_LENGTH);
            memcpy(header.token, gameClient.GetAuthToken().c_str(), tokenLength);
            gameClient.Send((uint8*)&header, sizeof(TokenPacketHeader), PacketParams::Reliable(PacketParams::TOKEN_CHANNEL_ID));
            gameClient.Update();
        }

        iterCount = 0;
        while (true)
        {
            TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
            udpServer.Update();
            bool allValidated = true;
            for (auto& gameClient : gameClients)
            {
                allValidated &= udpServer.HasResponder(gameClient.GetAuthToken());
                if (allValidated)
                {
                    udpServer.SetValidToken(gameClient.GetAuthToken());
                }
            }
            if (allValidated)
            {
                break;
            }
        }

        iterCount = 0;
        while (true)
        {
            TEST_VERIFY(iterCount++ < MAX_ITER_COUNT);
            bool allRecv = true;
            for (auto& client : gameClients)
            {
                auto rcv = rcvCount[client.idx];
                auto snd = sndCount[client.idx];
                if (rcv == MAX_PACKET_COUNT)
                {
                    continue;
                }

                allRecv = false;
                if (rcv == snd)
                {
                    client.Send((uint8*)&snd, sizeof(snd), DEF_PACKET_PARAMS);
                }
                client.Update();
            }
            if (allRecv)
            {
                break;
            }

            udpServer.Update();
        }

        TEST_VERIFY(rsdCount_ == MAX_PACKET_COUNT * NUM_CLIENTS);
        for (int i = 0; i < NUM_CLIENTS; ++i)
        {
            TEST_VERIFY(sndCount[i] == MAX_PACKET_COUNT);
            TEST_VERIFY(rcvCount[i] == MAX_PACKET_COUNT);
        }
    };
};


#endif // !defined(__DAVAENGINE_ANDROID__)
