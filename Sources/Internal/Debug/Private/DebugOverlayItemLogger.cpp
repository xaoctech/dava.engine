#pragma strict_gs_check(on)

#include "Debug/Private/DebugOverlayItemLogger.h"

#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "Concurrency/LockGuard.h"
#include "Concurrency/Mutex.h"
#include "Debug/DebugOverlay.h"
#include "Debug/Private/RingArray.h"
#include "Debug/Private/ImGui.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace DebugOverlayItemLoggerDetail
{
class LoggerOutputContainer : public LoggerOutput
{
public:
    void Output(Logger::eLogLevel level, const char8* text) override
    {
        LockGuard<Mutex> lock(mutex);
        logsArray.next() = Format("[%s] %s", Logger::GetLogLevelString(level), text);
    }

    RingArray<String> logsArray = RingArray<String>(256);
    Mutex mutex;
};
}

DebugOverlayItemLogger::DebugOverlayItemLogger()
    : loggerOutput{ new DebugOverlayItemLoggerDetail::LoggerOutputContainer }
    , collectingLogs(false)
{
}

DebugOverlayItemLogger::~DebugOverlayItemLogger()
{
    if (collectingLogs)
    {
        Logger::RemoveCustomOutput(loggerOutput.get());
    }
}

String DebugOverlayItemLogger::GetName() const
{
    return "Logger";
}

void DebugOverlayItemLogger::Draw()
{
    using namespace DebugOverlayItemLoggerDetail;

    bool shown = true;
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Logger", &shown, ImGuiWindowFlags_NoFocusOnAppearing))
    {
        static ImGuiTextFilter filter;

        bool value = collectingLogs;
        ImGui::Checkbox("Collect logs", &value);
        if (value != collectingLogs)
        {
            collectingLogs = value;

            if (collectingLogs)
            {
                Logger::AddCustomOutput(loggerOutput.get());
            }
            else
            {
                Logger::RemoveCustomOutput(loggerOutput.get());
                loggerOutput->logsArray = RingArray<String>(256); // clear
            }
        }
        ImGui::SameLine();
        filter.Draw();
        ImGui::Separator();

        ImGui::BeginChild("Scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        {
            LockGuard<Mutex> lock(loggerOutput->mutex);
            for (auto iter = loggerOutput->logsArray.rbegin(); iter != loggerOutput->logsArray.rend(); ++iter)
            {
                const String& s = *iter;
                if (!filter.IsActive() || filter.PassFilter(s.c_str()))
                {
                    ImGui::TextUnformatted(s.c_str());
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    if (!shown)
    {
        GetEngineContext()->debugOverlay->HideItem(this);
    }
}
}
