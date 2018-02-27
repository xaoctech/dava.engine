#include "NetworkCore/Private/NetworkCoreDebugOverlay.h"

#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Debug/DebugOverlay.h"
#include "Debug/Private/ImGui.h"

#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkClientSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkServerSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"

#include <Scene3D/Scene.h>
#include <Utils/FpsMeter.h>

namespace DAVA
{
String NetworkCoreDebugOverlayItem::GetName() const
{
    return "NetworkCore";
}

void NetworkCoreDebugOverlayItem::Draw(float32 elapsedTime)
{
    bool isShown = true;

    ImGui::SetNextWindowSizeConstraints(ImVec2(420.0f, 600.0f), ImVec2(FLOAT_MAX, FLOAT_MAX));
    if (ImGui::Begin("NetworkCore", &isShown, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        if (ImGui::CollapsingHeader("Snapshot misprediction"))
        {
            ImGui::Text("TODO NetworkCore");
        }

        if (ImGui::CollapsingHeader("NetworkTimeSystem") && scene != nullptr)
        {
            if (timeSingleComponent == nullptr)
            {
                timeSingleComponent = scene->GetSingletonComponent<NetworkTimeSingleComponent>();
            }
            uint32 frameId = timeSingleComponent->GetFrameId();

            static FpsMeter fpsMeter(1.f);
            fpsMeter.Update(elapsedTime);

            ImGui::Columns(2, "", false);
            ImGui::Text("FPS");
            ImGui::NextColumn();
            ImGui::Text("%.2f", fpsMeter.GetFps());
            ImGui::NextColumn();
            ImGui::Text("frequency");
            ImGui::NextColumn();
            ImGui::Text("%u Hz (%u ms)", NetworkTimeSingleComponent::FrequencyHz, NetworkTimeSingleComponent::FrameDurationMs);
            ImGui::NextColumn();
            ImGui::Text("uptime");
            ImGui::NextColumn();
            ImGui::Text("%u ms", timeSingleComponent->GetUptimeMs());
            ImGui::NextColumn();
            ImGui::Text("frameId");
            ImGui::NextColumn();
            ImGui::Text("%u", frameId);
            ImGui::NextColumn();
            ImGui::Columns(1);

            if (IsClient(scene))
            {
                ImGui::InputFloat("frame speedup", &NetworkTimeSingleComponent::FrameSpeedupS, 0.001f, 0.01f);
                ImGui::InputFloat("frame slowdown", &NetworkTimeSingleComponent::FrameSlowdownS, 0.00001f, 0.0001f);

                ImGui::Columns(2, "", false);
                int32 adjustedFrames = timeSingleComponent->GetAdjustedFrames();
                ImGui::Text(adjustedFrames > 0 ? "adjusted frames (slowdown)" :
                                                 adjustedFrames < 0 ? "adjusted frames (speedup)" :
                                                                      "adjusted frames");
                ImGui::NextColumn();
                ImGui::TextColored(adjustedFrames > 0 ? ImVec4(1.0f, 0.7f, 0.5f, 1.0f) :
                                                        adjustedFrames < 0 ? ImVec4(0.5f, 0.7f, 1.0f, 1.0f) :
                                                                             ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
                                   "%d (%.4f s)", adjustedFrames, abs(adjustedFrames) * NetworkTimeSingleComponent::FrameDurationS);
                ImGui::NextColumn();
                ImGui::Columns(1);
            }
            else
            {
                ImGui::InputFloat("resync init (xRTT)", &NetworkTimeSingleComponent::UptimeInitFactor, 0.01f, 0.1f);
                ImGui::InputFloat("resync delay (xRTT)", &NetworkTimeSingleComponent::UptimeDelayFactor, 0.1f, 1.0f);
                ImGui::InputInt("artificial latency", &NetworkTimeSingleComponent::ArtificialLatency, 1, 10);
                ImGui::InputFloat("loss factor", &NetworkTimeSingleComponent::LossFactor, 0.001f, 0.01f);
            }

            if (IsClient(scene))
            {
                if (client == nullptr)
                {
                    client = scene->GetSingletonComponent<NetworkClientSingleComponent>()->GetClient();
                }

                static float diffs[128] = { 0 };
                int32 diff = timeSingleComponent->GetLastSyncDiff();
                diffs[frameId % 128] = static_cast<float>(diff);
                ImGui::PlotLines("diff", diffs, 128, (frameId + 1) % 128, Format("%d", diff).c_str(), -10.f, 20.f, ImVec2(350, 150));

                static float fdiffs[128] = { 0 };
                int32 fdiff = timeSingleComponent->GetClientOutrunning(client->GetAuthToken());
                fdiffs[frameId % 128] = static_cast<float>(fdiff);
                ImGui::PlotLines("fdiff", fdiffs, 128, (frameId + 1) % 128, Format("%d", fdiff).c_str(), -10.f, 20.f, ImVec2(350, 150));
            }
            else
            {
                if (server == nullptr)
                {
                    server = scene->GetSingletonComponent<NetworkServerSingleComponent>()->GetServer();
                }

                if (gameModeSingleComponent == nullptr)
                {
                    gameModeSingleComponent = scene->GetSingletonComponent<NetworkGameModeSingleComponent>();
                }

                static UnorderedMap<FastName, Vector<float>> rttsByToken;
                static UnorderedMap<FastName, Vector<float>> packetLossByToken;
                static UnorderedMap<FastName, Vector<float>> fdiffsByToken;
                static UnorderedMap<FastName, Vector<float>> delaysByToken;

                server->Foreach([this, frameId](const Responder& responder)
                                {
                                    ImGui::Separator();
                                    const FastName& token = responder.GetToken();
                                    NetworkPlayerID playerID = gameModeSingleComponent->GetNetworkPlayerID(token);
                                    ImGui::Text("network player id %d", playerID);

                                    uint32 rtt = responder.GetRtt();
                                    PlotResponderHistory(rttsByToken, token, frameId, FastName("RTT"),
                                                         rtt, FastName(Format("%d ms", rtt)), { 0.f, 100.f });

                                    float32 packetLoss = responder.GetPacketLoss();
                                    PlotResponderHistory(packetLossByToken, token, frameId, FastName("loss"),
                                                         packetLoss, FastName(Format("%f %%", packetLoss * 100)), { 0.f, 100.f });

                                    int32 resyncDelay = timeSingleComponent->GetResyncDelayUptime(token) - timeSingleComponent->GetUptimeMs();
                                    if (resyncDelay > 0)
                                    {
                                        ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.7f, 1.0f), "resync delay %d ms", resyncDelay);
                                    }

                                    int32 fdiff = timeSingleComponent->GetClientOutrunning(token);
                                    PlotResponderHistory(fdiffsByToken, token, frameId, FastName("fdiff"),
                                                         fdiff, FastName(Format("%d", fdiff)), { -10.f, 20.f });

                                    int32 delay = timeSingleComponent->GetClientViewDelay(token,
                                                                                          frameId);
                                    PlotResponderHistory(delaysByToken, token, frameId, FastName("delay"),
                                                         delay, FastName(Format("%d", delay)), { -10.f, 20.f });
                                });
            }
        }
    }
    ImGui::End();

    if (!isShown)
    {
        GetEngineContext()->debugOverlay->HideItem(this);
    }

    // ImGui::ShowTestWindow();
}

void NetworkCoreDebugOverlayItem::PlotResponderHistory(
UnorderedMap<FastName, Vector<float>>& history, const FastName& token, uint32 frameId,
const FastName& name, float value, const FastName& valueStr, std::pair<float, float> domain)
{
    auto findIt = history.find(token);
    if (findIt != history.end())
    {
        findIt->second[frameId % 128] = value;
    }
    else
    {
        Vector<float> values(128, 0.f);
        values[frameId % 128] = value;
        history.emplace(token, values);
    }
    ImGui::PlotLines(name.c_str(), &*history[token].begin(), 128, (frameId + 1) % 128,
                     valueStr.c_str(), domain.first, domain.second, ImVec2(350, 50));
}
}
