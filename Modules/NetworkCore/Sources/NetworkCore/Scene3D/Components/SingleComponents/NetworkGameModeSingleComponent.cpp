#include "NetworkGameModeSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Logger/Logger.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkGameModeSingleComponent)
{
    ReflectionRegistrator<NetworkGameModeSingleComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("GameModeType", &NetworkGameModeSingleComponent::GetGameModeType, &NetworkGameModeSingleComponent::SetGameModeType)[M::Replicable()]
    .Field("MaxTokenCount", &NetworkGameModeSingleComponent::GetMaxTokenCount, &NetworkGameModeSingleComponent::SetMaxTokenCount)[M::Replicable()]
    .Field("TokenCount", &NetworkGameModeSingleComponent::GetTokenCount, &NetworkGameModeSingleComponent::SetTokenCount)[M::Replicable()]
    //    .Field("RawData", &NetworkGameModeSingleComponent::GetRawData, &NetworkGameModeSingleComponent::SetRawData)[M::Replicable()]
    .Field("MapName", &NetworkGameModeSingleComponent::GetMapName, &NetworkGameModeSingleComponent::SetMapName)[M::Replicable()]
    .End();
}

void NetworkGameModeSingleComponent::SetGameModeType(int32 gameModeType_)
{
    gameModeType = gameModeType_;
}

int32 NetworkGameModeSingleComponent::GetGameModeType() const
{
    return gameModeType;
}

void NetworkGameModeSingleComponent::SetRawData(const FixedVector<uint8>& rawData_)
{
    rawData = rawData_;
}

FixedVector<uint8>& NetworkGameModeSingleComponent::GetRawData()
{
    return rawData;
}

void NetworkGameModeSingleComponent::AddConnectedToken(const FastName& token)
{
    presentTokens.insert(token);
    connectedTokens.push_back(token);
}

void NetworkGameModeSingleComponent::RemoveConnectedToken(const FastName& token)
{
    presentTokens.erase(token);
    readyTokens.erase(token);
    SetTokenCount(static_cast<uint8>(readyTokens.size()));
}

void NetworkGameModeSingleComponent::AddReadyToken(const FastName& token)
{
    if (presentTokens.find(token) != presentTokens.end())
    {
        readyTokens.insert(token);
        loadedTokens.push_back(token);
        SetTokenCount(static_cast<uint8>(readyTokens.size()));
        return;
    }
    DVASSERT(false);
}

const UnorderedSet<FastName>& NetworkGameModeSingleComponent::GetValidTokens() const
{
    return validTokens;
}

const UnorderedSet<FastName>& NetworkGameModeSingleComponent::GetPresentTokens() const
{
    return presentTokens;
}

const UnorderedSet<FastName>& NetworkGameModeSingleComponent::GetReadyTokens() const
{
    return readyTokens;
}

const Vector<FastName>& NetworkGameModeSingleComponent::GetConnectedTokens() const
{
    return connectedTokens;
}

void NetworkGameModeSingleComponent::ClearConnectedTokens()
{
    connectedTokens.clear();
}

const Vector<FastName>& NetworkGameModeSingleComponent::GetLoadedTokens() const
{
    return loadedTokens;
}

void NetworkGameModeSingleComponent::ClearLoadedTokens()
{
    loadedTokens.clear();
}

void NetworkGameModeSingleComponent::AddValidToken(const FastName& token)
{
    if (validTokens.insert(token).second)
    {
        SetMaxTokenCount(static_cast<uint8>(validTokens.size()));
    }
}

NetworkPlayerID NetworkGameModeSingleComponent::GetNextNetworkPlayerID()
{
    DVASSERT(nextPlayerID < MAX_NETWORK_PLAYERS_COUNT);
    ++nextPlayerID;
    return nextPlayerID;
}

void NetworkGameModeSingleComponent::AddNetworkPlayerID(const FastName& token, NetworkPlayerID networkPlayerID)
{
    tokensToPlayerIDs.emplace(token, networkPlayerID);
    playerIDsToTokens.emplace(networkPlayerID, token);
    playerIds.push_back(networkPlayerID);
}

NetworkPlayerID NetworkGameModeSingleComponent::GetNetworkPlayerID(const FastName& token) const
{
    NetworkPlayerID result = 0;
    auto findIt = tokensToPlayerIDs.find(token);
    if (findIt != tokensToPlayerIDs.end())
    {
        result = findIt->second;
    }
    return result;
}

const FastName& NetworkGameModeSingleComponent::GetToken(NetworkPlayerID networkPlayerID) const
{
    auto findIt = playerIDsToTokens.find(networkPlayerID);
    DVASSERT(findIt != playerIDsToTokens.end());
    return findIt->second;
}

void NetworkGameModeSingleComponent::SetMaxTokenCount(uint8 value)
{
    maxTokenCount = value;
}

uint8 NetworkGameModeSingleComponent::GetMaxTokenCount() const
{
    return maxTokenCount;
}

void NetworkGameModeSingleComponent::SetTokenCount(uint8 value)
{
    tokenCount = value;
}

uint8 NetworkGameModeSingleComponent::GetTokenCount() const
{
    return tokenCount;
}

void NetworkGameModeSingleComponent::SetMapName(FastName mapName_)
{
    mapName = mapName_;
}

FastName NetworkGameModeSingleComponent::GetMapName() const
{
    return mapName;
}

void NetworkGameModeSingleComponent::SetIsLoaded(bool value)
{
    isLoaded = value;
}

bool NetworkGameModeSingleComponent::IsLoaded() const
{
    return isLoaded;
}

void NetworkGameModeSingleComponent::SetNetworkPlayerID(NetworkPlayerID playerID)
{
    networkPlayerID = playerID;
}

NetworkPlayerID NetworkGameModeSingleComponent::GetNetworkPlayerID() const
{
    return networkPlayerID;
}

void NetworkGameModeSingleComponent::AddPlayerEnity(NetworkPlayerID networkPlayerID, Entity* entity)
{
    playerIDsToEntities.emplace(networkPlayerID, entity);
}

Entity* NetworkGameModeSingleComponent::GetPlayerEnity(NetworkPlayerID networkPlayerID) const
{
    auto findIt = playerIDsToEntities.find(networkPlayerID);
    if (findIt == playerIDsToEntities.end())
    {
        return nullptr;
    }
    return findIt->second;
}

void NetworkGameModeSingleComponent::RemovePlayerEntity(NetworkPlayerID networkPlayerID)
{
    playerIDsToEntities.erase(networkPlayerID);
}

const Vector<NetworkPlayerID>& NetworkGameModeSingleComponent::GetPlayerIds() const
{
    return playerIds;
}

void NetworkGameModeSingleComponent::Clear()
{
}
}
