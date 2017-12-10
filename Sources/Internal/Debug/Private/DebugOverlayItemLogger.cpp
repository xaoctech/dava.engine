#pragma strict_gs_check(on)

#include "Debug/Private/DebugOverlayItemLogger.h"

#include "Engine/Engine.h"
#include "Engine/EngineSettings.h"
#include "Debug/DebugOverlay.h"
#include "Debug/Private/ImGui.h"
#include "Debug/Private/RingArray.h"
#include "Logger/Logger.h"

#include <sstream>

namespace DAVA
{
namespace DebugOverlayItemLoggerDetail
{
class LoggerOutputContainer : public LoggerOutput
{
public:
    void Output(Logger::eLogLevel level, const char8* text) override
    {
        logsArray.next() = Format("[%s] %s", Logger::GetLogLevelString(level), text);
    }

    RingArray<String> logsArray = RingArray<String>(256);
};
}

DebugOverlayItemLogger::DebugOverlayItemLogger()
    : loggerOutput{ new DebugOverlayItemLoggerDetail::LoggerOutputContainer }
{
    Logger::AddCustomOutput(loggerOutput.get());
}

DebugOverlayItemLogger::~DebugOverlayItemLogger()
{
    Logger::RemoveCustomOutput(loggerOutput.get());
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
        filter.Draw();

        ImGui::Separator();

        ImGui::BeginChild("Scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (auto iter = loggerOutput->logsArray.rbegin(); iter != loggerOutput->logsArray.rend(); ++iter)
        {
            const String& s = *iter;
            if (!filter.IsActive() || filter.PassFilter(s.c_str()))
            {
                ImGui::TextUnformatted(s.c_str());
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