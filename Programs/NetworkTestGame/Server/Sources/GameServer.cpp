#include "GameServer.h"

#include "Scene3D/Scene.h"
#include "Time/SystemTimer.h"
#include "Logger/Logger.h"

#include <Debug/ProfilerCPU.h>
#include <Debug/ProfilerGPU.h>
#include <Debug/ProfilerUtils.h>

#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkPlayerComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"

using namespace DAVA;

namespace GameServerDetail
{
static const int32 MAX_UPDATE_DURATION_US = 4000;
}

GameServer::GameServer(uint32 host, uint16 port, uint8 clientsNumber_)
    : clientsNumber(clientsNumber_)
    , udpServer(host, port, clientsNumber)
{
}

GameServer::~GameServer()
{
    if (dumpThread)
    {
        dumpThread->Cancel();
        dumpThread->Join();
        dumpThread->Release();
        SafeRelease(dumpThread);
    }
}

void GameServer::UpdateConsoleMode()
{
    int64 delta = 0;
    if (frameTimeUs > 0)
    {
        delta = SystemTimer::GetUs() - frameTimeUs;
    }

    static uint8 exceedCounter = 0;
    if (delta > NetworkTimeSingleComponent::FrameDurationUs)
    {
        ++exceedCounter;
        if (exceedCounter >= 4)
            SetPoisonPill(4);

        Logger::Error("Last frame duration exceeded limits: %lld", delta);
        delta = 0;
    }
    {
        DAVA_PROFILER_CPU_SCOPE("while(udpServer.Update())");
        int64 beginUs = SystemTimer::GetUs();
        while (SystemTimer::GetUs() - beginUs < NetworkTimeSingleComponent::FrameDurationUs - delta)
        {
            udpServer.Update();
        }
    }
    frameTimeUs = SystemTimer::GetUs();

    if (poisonPill.isActive)
    {
        ++poisonPill.framesAfterActive;
        if (poisonPill.IsKill())
        {
            Logger::Error("Application was killed by SetPoisonPill(%lu)", poisonPill.framesBeforeKill);
            ProfilerCPU::globalProfiler->Stop();
            ProfilerGPU::globalProfiler->Stop();
            ProfilerUtils::DumpCPUGPUTraceToFile(ProfilerCPU::globalProfiler, ProfilerGPU::globalProfiler,
                                                 opts.profilePath);
            exit(1);
        }
    }
}

void GameServer::UpdateGUIMode()
{
    using namespace GameServerDetail;
    int64 updUs = SystemTimer::GetUs();
    while (udpServer.Update())
    {
        if (SystemTimer::GetUs() - updUs > MAX_UPDATE_DURATION_US)
        {
            break;
        }
    }
}

void GameServer::Update(DAVA::float32 timeElapsed)
{
    if (!opts.profilePath.empty() && !opts.hasPoisonPill)
    {
        static const float32 dumpCycle = 5.f;
        static float32 cycleTime = dumpCycle;
        cycleTime -= timeElapsed;
        if (cycleTime < 0.f)
        {
            cycleTime = dumpCycle;
            std::unique_lock<std::mutex> guard(lock);
            ProfilerCPU::globalProfiler->Stop();
            ProfilerGPU::globalProfiler->Stop();
            ProfilerCPU::globalProfiler->DeleteSnapshots();
            ProfilerCPU::globalProfiler->MakeSnapshot();
            ProfilerCPU::globalProfiler->Start();
            ProfilerGPU::globalProfiler->Start();
            hasProducedMetrics = true;
            notifier.notify_one();
        }
    }

    DAVA_PROFILER_CPU_SCOPE("GameServer::Update");

    if (opts.withGUI)
    {
        UpdateGUIMode();
    }
    else
    {
        UpdateConsoleMode();
    }
}

void GameServer::Setup(const GameServer::Options& opts_)
{
    opts = opts_;
    if (!opts.profilePath.empty() && !opts.hasPoisonPill)
    {
        dumpThread = Thread::Create([this] {
            Thread* thread = Thread::Current();
            do
            {
                std::unique_lock<std::mutex> guard(lock);
                notifier.wait(guard, [this]() { return hasProducedMetrics; });
                TraceEvent::DumpJSON(ProfilerCPU::globalProfiler->GetTrace(0), opts.profilePath);
                hasProducedMetrics = false;
            } while (!thread->IsCancelling());
        });
        dumpThread->SetName("ProfileDumpThread");
        dumpThread->Start();
    }
}

void GameServer::SetPoisonPill(uint32 framesBeforeKill)
{
    if (!poisonPill.isActive && opts.hasPoisonPill)
    {
        poisonPill.isActive = true;
        poisonPill.framesBeforeKill = framesBeforeKill;
    }
}

UDPServer& GameServer::GetUDPServer()
{
    return udpServer;
}
