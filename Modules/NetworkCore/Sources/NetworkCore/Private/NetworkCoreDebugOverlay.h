#pragma once

#include <Debug/DebugOverlay.h>
#include <Debug/DebugOverlayItem.h>

#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/UDPTransport/UDPClient.h"
#include "NetworkCore/UDPTransport/UDPServer.h"

namespace DAVA
{
class NetworkCoreDebugOverlayItem final : public DebugOverlayItem
{
public:
    String GetName() const override;
    void Draw(bool* shown, float32 timeElapsed) override;

private:
    void PlotResponderHistory(UnorderedMap<FastName, Vector<float>>& history, const FastName& token, uint32 frameId,
                              const FastName& name, float value, const FastName& valueStr, std::pair<float, float> domain);

    IClient* client = nullptr;
    IServer* server = nullptr;
    NetworkTimeSingleComponent* timeSingleComponent = nullptr;
    NetworkGameModeSingleComponent* gameModeSingleComponent = nullptr;
};
}
