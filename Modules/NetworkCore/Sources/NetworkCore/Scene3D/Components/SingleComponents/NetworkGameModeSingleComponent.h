#pragma once

#include "Base/UnordererSet.h"
#include "Base/Vector.h"
#include "Base/FastName.h"
#include "Entity/SingletonComponent.h"
#include "Reflection/Reflection.h"
#include <Base/FixedVector.h>
#include <NetworkCore/NetworkTypes.h>

namespace DAVA
{
class NetworkGameModeSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkGameModeSingleComponent, SingletonComponent);

    void SetGameModeType(int32 gameModeType_);
    int32 GetGameModeType() const;
    void SetRawData(const FixedVector<uint8>& rawData_);
    FixedVector<uint8>& GetRawData();
    void SetMapName(FastName mapName_);
    FastName GetMapName() const;
    void SetIsLoaded(bool value);
    bool IsLoaded() const;

    // server usage
    void AddConnectedToken(const FastName& token);
    void RemoveConnectedToken(const FastName& token);

    void AddReadyToken(const FastName& token);

    const UnorderedSet<FastName>& GetValidTokens() const;
    const UnorderedSet<FastName>& GetPresentTokens() const;
    const UnorderedSet<FastName>& GetReadyTokens() const;

    const Vector<FastName>& GetConnectedTokens() const;
    void ClearConnectedTokens();

    const Vector<FastName>& GetLoadedTokens() const;
    void ClearLoadedTokens();

    void AddValidToken(const FastName& token);
    NetworkPlayerID GetNextNetworkPlayerID();
    void AddNetworkPlayerID(const FastName& token, NetworkPlayerID networkPlayerID);
    NetworkPlayerID GetNetworkPlayerID(const FastName& token) const;
    const FastName& GetToken(NetworkPlayerID networkPlayerID) const;

    void AddPlayerEnity(NetworkPlayerID networkPlayerID, Entity* entity);
    Entity* GetPlayerEnity(NetworkPlayerID networkPlayerID) const;
    void RemovePlayerEntity(NetworkPlayerID networkPlayerID);

    // client usage
    void SetMaxTokenCount(uint8 value);
    uint8 GetMaxTokenCount() const;
    void SetTokenCount(uint8 value);
    uint8 GetTokenCount() const;
    void SetNetworkPlayerID(NetworkPlayerID playerID);
    NetworkPlayerID GetNetworkPlayerID() const;

    const Vector<NetworkPlayerID>& GetPlayerIds() const;

private:
    NetworkPlayerID nextPlayerID = 0;
    UnorderedMap<FastName, NetworkPlayerID> tokensToPlayerIDs;
    UnorderedMap<NetworkPlayerID, FastName> playerIDsToTokens;
    UnorderedMap<NetworkPlayerID, Entity*> playerIDsToEntities;
    Vector<NetworkPlayerID> playerIds;

    // server can accept these tokens
    UnorderedSet<FastName> validTokens;
    // connected tokens to the server
    UnorderedSet<FastName> presentTokens;
    // server accepted these tokens
    UnorderedSet<FastName> readyTokens;

    // new connected tokens
    Vector<FastName> connectedTokens;
    // tokens which finished loading process
    Vector<FastName> loadedTokens;

    uint8 maxTokenCount = 0;
    uint8 tokenCount = 0;

    int32 gameModeType = 0;
    FixedVector<uint8> rawData = { 10 };
    FastName mapName;

    bool isLoaded = false;
    NetworkPlayerID networkPlayerID = 0;

    void Clear() override;
};
}
