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
            void Output(Logger::eLogLevel ll, const char8* text)
            {
                std::stringstream stream;
                stream << "[" << LogLevelToString(ll) << "] " << text;
                String result = stream.str();
                logsArray.next() = std::move(result);

                stream.str("");
                for (auto iter = logsArray.rbegin(); iter != logsArray.rend(); ++iter)
                {
                    stream << *iter;
                }
                str = stream.str();
            }

            String GetStr()
            {
                return str;
            }

            String LogLevelToString(Logger::eLogLevel level)
            {
                if (level == Logger::eLogLevel::LEVEL_DEBUG)
                {
                    return "Debug";
                }
                else if (level == Logger::eLogLevel::LEVEL_ERROR)
                {
                    return "Error";
                }
                else if (level == Logger::eLogLevel::LEVEL_FRAMEWORK)
                {
                    return "Framework";
                }
                else if (level == Logger::eLogLevel::LEVEL_INFO)
                {
                    return "Info";
                }
                else if (level == Logger::eLogLevel::LEVEL_WARNING)
                {
                    return "Warning";
                }

                DVASSERT(false);
                return "Unknown";
            }

            String str;
            RingArray<String> logsArray = RingArray<String>(256);
        };
    }

    static DebugOverlayItemLoggerDetail::LoggerOutputContainer loggerOutput;

    DebugOverlayItemLogger::DebugOverlayItemLogger()
    {
        Logger::AddCustomOutput(&loggerOutput);
    }

    DebugOverlayItemLogger::~DebugOverlayItemLogger()
    {
        Logger::RemoveCustomOutput(&loggerOutput);
    }

    String DebugOverlayItemLogger::GetName()
    {
        return "Logger";
    }

    void DebugOverlayItemLogger::Draw()
    {
        static ImGuiTextFilter filter;

        bool shown = true;
        ImGui::SetNextWindowSize(ImVec2(800, 500), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Logger", &shown);

        /*if (ImGui::Button("Clear"))
        {

        }
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        filter.Draw("Filter", -100.0f);
        ImGui::Separator();*/

        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        /*if (filter.IsActive())
        {
            for (auto iter = loggerOutput.logsArray.rbegin(); iter != loggerOutput.logsArray.rend(); ++iter)
            {
                const String& s = *iter;
                if (filter.PassFilter(s.c_str()))
                {
                    ImGui::TextUnformatted(s.c_str());
                }
            }
        }
        else*/
        {
                ImGui::TextUnformatted(loggerOutput.str.c_str());
        }

        ImGui::EndChild();

        ImGui::End();
        if (!shown)
        {
            GetEngineContext()->debugOverlay->DisableItem(this);
        }
    }
}