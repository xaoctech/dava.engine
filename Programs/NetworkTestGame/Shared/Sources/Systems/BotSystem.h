#pragma once

#include <Base/BaseTypes.h>
#include <Base/Vector.h>
#include <Entity/SceneSystem.h>
#include <NetworkCore/UDPTransport/UDPServer.h>

#include <random>

namespace DAVA
{
class Scene;
class ActionsSingleComponent;
}

class BotResponder : public DAVA::Responder
{
public:
    BotResponder(const DAVA::FastName& token)
        : token(token)
    {
    }

    void Send(const DAVA::uint8* data, size_t size, const DAVA::PacketParams& param) const override
    {
    }
    DAVA::uint32 GetRtt() const override
    {
        return 0;
    }
    DAVA::float32 GetPacketLoss() const override
    {
        return 0.f;
    }
    const DAVA::FastName& GetToken() const override
    {
        return token;
    }
    void SetToken(const DAVA::FastName& token_) override
    {
        token = token_;
    }
    bool IsValid() const override
    {
        return true;
    }
    void SetIsValid(bool value) override
    {
    }
    const DAVA::uint8 GetTeamID() const override
    {
        return 0;
    }
    void SetTeamID(DAVA::uint8 teamID_) override
    {
    }
    ENetPeer* GetPeer() const override
    {
        return nullptr;
    }
    void SaveRtt() override
    {
    }
    bool RttIsBetter() const override
    {
        return false;
    }

    ~BotResponder() override
    {
    }

    DAVA::uint32 startTimeout = 0;
    bool turnLeft = false;

private:
    DAVA::FastName token;
};

class BotSystem : public DAVA::SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(BotSystem, DAVA::SceneSystem);

    BotSystem(DAVA::Scene* scene);
    const DAVA::Vector<BotResponder>& GetBotResponders() const;
    void ProcessFixed(DAVA::float32 timeElapsed) override;
    void ProcessServer(DAVA::float32 timeElapsed);
    void ProcessClient(DAVA::float32 timeElapsed);
    void PrepareForRemove() override{};
    void SetBotsCount(DAVA::int32 botsCount);

private:
    std::mt19937 randEngine;
    bool turnLeft = false;
    DAVA::float32 warmupDuration = 1.f;
    DAVA::Vector<BotResponder> botResponders;
    DAVA::ActionsSingleComponent* actionsSingleComponent = nullptr;

    bool GenNextTurn();
    void GenerateDigitalActions(DAVA::Vector<DAVA::FastName>& digitalActions, bool turnState);
};
