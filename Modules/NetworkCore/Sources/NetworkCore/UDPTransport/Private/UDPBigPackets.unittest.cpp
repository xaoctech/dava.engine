#include "UnitTests/UnitTests.h"
#include "Engine/Engine.h"
#include "Base/Exception.h"

#if !defined(__DAVAENGINE_ANDROID__)

#include <memory>
#include <string.h>

#include "NetworkCore/UDPTransport/UDPServer.h"
#include "NetworkCore/UDPTransport/UDPClient.h"

using namespace DAVA;

DAVA_TESTCLASS (UDPBigPacketsTest)
{
    static const size_t MAX_PACKET_SIZE = 4096;
    bool isDone = false;

    static void OnConnectServer(const Responder& responder)
    {
        PacketParams params = PacketParams::Reliable(PacketParams::DEFAULT_CHANNEL_ID);
        std::unique_ptr<uint8[]> data(new uint8[MAX_PACKET_SIZE]);
        for (uint32 i = 0; i < MAX_PACKET_SIZE; i += sizeof(uint32))
        {
            memcpy(data.get() + i, &i, sizeof(uint32));
        }
        responder.Send(data.get(), MAX_PACKET_SIZE, params);
    }

    void OnReceiveClient(const uint8* data, size_t size, uint8, uint32)
    {
        TEST_VERIFY(size == MAX_PACKET_SIZE);
        for (uint32 i = 0; i < size; i += sizeof(uint32))
        {
            int32 x = 0;
            memcpy(&x, data + i, sizeof(uint32));
            TEST_VERIFY(x == i);
        }
        isDone = true;
    }

    DAVA_TEST (SendPacket4k)
    {
        UDPServer server(0, 10101, 1);

        struct Storage : INetworkEventStorage
        {
            Storage(UDPServer& server_)
                : server(server_)
            {
            }
            void AddConnectedToken(const FastName& token) final
            {
            }
            void StoreRecvPacket(const PacketParams::Channels channel, const FastName& token, const void* dataPtr, size_t dataSize) final
            {
            }
            void RemoveConnectedToken(const FastName& token) final
            {
            }
            void ConfirmToken(const FastName& token) final
            {
                const Responder& responder = server.GetResponder(token);
                OnConnectServer(responder);
            }

        private:
            UDPServer& server;
        } storage(server);

        server.SetNetworkEventStorage(storage);

        //server.SubscribeOnConnect(OnServerConnectCb(this, &UDPBigPacketsTest::OnConnectServer));
        FastName token(Format("%064d", 1));
        UDPClient client("localhost", 10101, token, 1);
        client.SubscribeOnReceive(PacketParams::DEFAULT_CHANNEL_ID,
                                  OnClientReceiveCb(this, &UDPBigPacketsTest::OnReceiveClient));

        int32 iterCount = 1000;
        while (!client.IsConnected())
        {
            TEST_VERIFY(iterCount-- > 0);
            client.Update();
            server.Update();
        }
        TokenPacketHeader header;
        uint32 tokenLength = static_cast<uint32>(token.size());
        DVASSERT(tokenLength == TokenPacketHeader::TOKEN_LENGTH);
        memcpy(header.token, token.c_str(), tokenLength);
        client.Send((uint8*)&header, sizeof(TokenPacketHeader),
                    PacketParams::Reliable(PacketParams::TOKEN_CHANNEL_ID));
        iterCount = 1000;
        while (!server.HasResponder(token))
        {
            TEST_VERIFY(iterCount-- > 0);
            client.Update();
            server.Update();
        }
        server.SetValidToken(token);

        iterCount = 1000;
        while (iterCount > 0)
        {
            server.Update();
            client.Update();
            --iterCount;
        }
        TEST_VERIFY(isDone);
    }
};

#endif // !defined(__DAVAENGINE_ANDROID__)
