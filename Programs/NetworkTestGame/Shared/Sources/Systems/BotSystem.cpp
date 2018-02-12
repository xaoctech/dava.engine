#include "BotSystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/SingleComponents/ActionsSingleComponent.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>
#include <NetworkCore/NetworkCoreUtils.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(BotSystem)
{
    ReflectionRegistrator<BotSystem>::Begin()[M::Tags("bot", "randombot")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessClient", &BotSystem::ProcessClient)[M::SystemProcess(SP::Group::ENGINE_BEGIN, SP::Type::FIXED, 4.3f)]
    .Method("ProcessServer", &BotSystem::ProcessServer)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 4.0f)]
    .End();
}

namespace BotSystemDetail
{
static const float32 DEFAULT_CHANGE_KEY_CHANCE = 0.001f;
}

BotSystem::BotSystem(Scene* scene)
    : SceneSystem(scene, 0)
    , randEngine(111) // every time seed with the same value

{
    actionsSingleComponent = scene->GetSingletonComponent<ActionsSingleComponent>();
}

const Vector<BotResponder>& BotSystem::GetBotResponders() const
{
    return botResponders;
}

void BotSystem::ProcessFixed(DAVA::float32 timeElapsed)
{
    warmupDuration -= timeElapsed;
    if (warmupDuration > 0.f)
    {
        return;
    }
    Scene* scene = GetScene();
    auto clientEmitInput = [this](const Vector<FastName>& digitalActions)
    {
        for (const FastName& action : digitalActions)
        {
            actionsSingleComponent->AddDigitalAction(action, actionsSingleComponent->GetLocalPlayerId());
        }
    };
    auto serverEmitInput = [scene](Scene* scene, Entity* entity, const Vector<FastName>& digitalActions)
    {
        ActionsSingleComponent::Actions actions;
        actions.digitalActions = digitalActions;
        AddActionsForClient(scene, entity, std::move(actions));
    };
    Vector<FastName> digitalActions;
    if (IsServer(this))
    {
        NetworkGameModeSingleComponent* netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
        DVASSERT(netGameModeComp);
        for (BotResponder& responder : botResponders)
        {
            if (responder.startTimeout > 0)
            {
                responder.startTimeout--;
            }
            else
            {
                bool needTurn = BotSystem::GenNextTurn();
                if (needTurn)
                {
                    responder.turnLeft = !responder.turnLeft;
                }

                GenerateDigitalActions(digitalActions, responder.turnLeft);
                const FastName& token = responder.GetToken();
                Entity* entity = netGameModeComp->GetPlayerEnity(netGameModeComp->GetNetworkPlayerID(token));
                if (entity)
                {
                    serverEmitInput(GetScene(), entity, digitalActions);
                }
                digitalActions.clear();
            }
        }
    }
    else
    {
        bool needTurn = BotSystem::GenNextTurn();
        if (needTurn)
        {
            turnLeft = !turnLeft;
        }

        GenerateDigitalActions(digitalActions, turnLeft);
        clientEmitInput(digitalActions);
    }
}

void BotSystem::ProcessServer(DAVA::float32 timeElapsed)
{
    if (!IsServer(GetScene()))
    {
        return;
    }

    warmupDuration -= timeElapsed;
    if (warmupDuration > 0.f)
    {
        return;
    }
    Scene* scene = GetScene();
    Vector<FastName> digitalActions;
    auto serverEmitInput = [scene](Scene* scene, Entity* entity, const Vector<FastName>& digitalActions)
    {
        ActionsSingleComponent::Actions actions;
        actions.digitalActions = digitalActions;
        AddActionsForClient(scene, entity, std::move(actions));
    };
    NetworkGameModeSingleComponent* netGameModeComp = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
    for (BotResponder& responder : botResponders)
    {
        if (responder.startTimeout > 0)
        {
            responder.startTimeout--;
        }
        else
        {
            bool needTurn = BotSystem::GenNextTurn();
            if (needTurn)
            {
                responder.turnLeft = !responder.turnLeft;
            }

            GenerateDigitalActions(digitalActions, responder.turnLeft);
            const FastName& token = responder.GetToken();
            Entity* entity = netGameModeComp->GetPlayerEnity(netGameModeComp->GetNetworkPlayerID(token));
            if (entity)
            {
                serverEmitInput(GetScene(), entity, digitalActions);
            }
            digitalActions.clear();
        }
    }
}

void BotSystem::ProcessClient(DAVA::float32 timeElapsed)
{
    if (!IsClient(GetScene()))
    {
        return;
    }

    warmupDuration -= timeElapsed;
    if (warmupDuration > 0.f)
    {
        return;
    }
    Scene* scene = GetScene();
    Vector<FastName> digitalActions;
    auto clientEmitInput = [this](const Vector<FastName>& digitalActions)
    {
        for (const FastName& action : digitalActions)
        {
            actionsSingleComponent->AddDigitalAction(action, actionsSingleComponent->GetLocalPlayerId());
        }
    };

    bool needTurn = BotSystem::GenNextTurn();
    if (needTurn)
    {
        turnLeft = !turnLeft;
    }

    GenerateDigitalActions(digitalActions, turnLeft);
    clientEmitInput(digitalActions);
}

void BotSystem::SetBotsCount(DAVA::int32 botsCount)
{
    static std::uniform_int_distribution<> disStartTime(0, 1000);
    static std::uniform_int_distribution<> disTurn(0, 1);

    int32 host = 0;
    for (int32 i = 0; i < botsCount; ++i)
    {
        int32 botId = i % 60;
        if (botId == 0)
        {
            ++host;
        }

        FastName botToken(Format("%064d", (botId + 1) * 10 + host));
        botResponders.emplace_back(botToken);
        botResponders.back().startTimeout = disStartTime(randEngine);
        botResponders.back().turnLeft = (disTurn(randEngine) > 0);
    }
}

bool BotSystem::GenNextTurn()
{
    static std::uniform_real_distribution<> dis(0.0, 1.0);

    bool ret = false;

    DAVA::float32 rand = static_cast<DAVA::float32>(dis(randEngine));
    if (rand < BotSystemDetail::DEFAULT_CHANGE_KEY_CHANCE)
    {
        ret = true;
    }

    return ret;
}

void BotSystem::GenerateDigitalActions(DAVA::Vector<FastName>& digitalActions, bool turnState)
{
    digitalActions.push_back(FastName("UP"));
    if (turnState)
    {
        digitalActions.push_back(FastName("LEFT"));
    }
    else
    {
        digitalActions.push_back(FastName("RIGHT"));
    }
}
