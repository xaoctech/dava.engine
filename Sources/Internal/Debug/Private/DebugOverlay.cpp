#include "Debug/DebugOverlay.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Engine/Window.h"
#include "Concurrency/Thread.h"
#include "Debug/Private/ImGui.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

#include "Debug/DebugOverlayItem.h"
#include "Debug/Private/DebugOverlayItemEngineSettings.h"
#include "Debug/Private/DebugOverlayItemLogger.h"
#include "Debug/Private/DebugOverlayItemRenderOptions.h"
#include "Debug/Private/DebugOverlayItemRenderStats.h"
#include "Debug/Private/DebugOverlayItemProfiler.h"

namespace DAVA
{
DebugOverlay::DebugOverlay()
    : defaultItemEngineSettings{ new DebugOverlayItemEngineSettings }
    , defaultItemLogger{ new DebugOverlayItemLogger }
    , defaultItemRenderOptions{ new DebugOverlayItemRenderOptions }
    , defaultItemRenderStats{ new DebugOverlayItemRenderStats }
    , defaultItemProfiler{ new DebugOverlayItemProfiler(ProfilerGPU::globalProfiler, ProfilerCPU::globalProfiler, ProfilerCPUMarkerName::ENGINE_ON_FRAME) }
{
    RegisterDefaultItems();
}

DebugOverlay::~DebugOverlay()
{
    UnregisterDefaultItems();

    DVASSERT(items.size() == 0);

    if (shown)
    {
        Engine::Instance()->update.Disconnect(this);
    }
}

void DebugOverlay::Show()
{
    DVASSERT(Thread::IsMainThread());

    if (!shown)
    {
        Window* primaryWindow = GetPrimaryWindow();
        DVASSERT(primaryWindow != nullptr);
        primaryWindow->update.Connect(this, &DebugOverlay::OnUpdate);

        shown = true;
    }
}

void DebugOverlay::Hide()
{
    DVASSERT(Thread::IsMainThread());

    if (shown)
    {
        Window* primaryWindow = GetPrimaryWindow();
        DVASSERT(primaryWindow != nullptr);
        primaryWindow->update.Disconnect(this);

        shown = false;
    }
}

bool DebugOverlay::IsShown() const
{
    DVASSERT(Thread::IsMainThread());

    return shown;
}

void DebugOverlay::RegisterItem(DebugOverlayItem* overlayItem)
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(overlayItem != nullptr);
    DVASSERT(std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) { return itemData.item == overlayItem; }) == items.end());

    ItemData itemData;
    itemData.item = overlayItem;
    itemData.name = overlayItem->GetName();
    itemData.shown = false;

    items.push_back(std::move(itemData));
}

void DebugOverlay::UnregisterItem(DebugOverlayItem* overlayItem)
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(overlayItem != nullptr);

    auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) { return itemData.item == overlayItem; });
    DVASSERT(iter != items.end());

    if (iter->shown)
    {
        iter->item->OnHidden();
    }

    items.erase(iter);
}

void DebugOverlay::ShowItem(DebugOverlayItem* overlayItem)
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(overlayItem != nullptr);

    auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) { return itemData.item == overlayItem; });
    DVASSERT(iter != items.end());

    if (iter->shown == false)
    {
        iter->shown = true;
        iter->item->OnShown();
    }
}

void DebugOverlay::HideItem(DebugOverlayItem* overlayItem)
{
    DVASSERT(Thread::IsMainThread());
    DVASSERT(overlayItem != nullptr);

    auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) { return itemData.item == overlayItem; });
    DVASSERT(iter != items.end());

    if (iter->shown == true)
    {
        iter->shown = false;
        iter->item->OnHidden();
    }
}

void DebugOverlay::RegisterDefaultItems()
{
    RegisterItem(defaultItemEngineSettings.get());
    RegisterItem(defaultItemLogger.get());
    RegisterItem(defaultItemRenderOptions.get());
    RegisterItem(defaultItemRenderStats.get());
    RegisterItem(defaultItemProfiler.get());
}

void DebugOverlay::UnregisterDefaultItems()
{
    UnregisterItem(defaultItemEngineSettings.get());
    UnregisterItem(defaultItemLogger.get());
    UnregisterItem(defaultItemRenderOptions.get());
    UnregisterItem(defaultItemRenderStats.get());
    UnregisterItem(defaultItemProfiler.get());
}

void DebugOverlay::OnUpdate(Window* window, float32 timeDelta)
{
    DVASSERT(ImGui::IsInitialized());

    auto PushColor = [](float32 mainH) {
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(mainH, 1.f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(mainH, 1.f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(mainH, 1.f, 0.8f)));
    };

    auto PopColor = []() {
        ImGui::PopStyleColor(3);
    };

    if (ImGui::IsInitialized())
    {
        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));
        uint32 windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("DebugOverlayWindow", nullptr, windowFlags))
        {
            if (ImGui::Button("Debug views"))
            {
                ImGui::OpenPopup("DebugViewsPopup");
            }

            ImGui::SetNextWindowPos(ImVec2(20.0f, ImGui::GetWindowSize().y + 20.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
            if (ImGui::BeginPopup("DebugViewsPopup"))
            {
                for (ItemData& itemData : items)
                {
                    const bool wasShown = itemData.shown;
                    ImGui::Checkbox(itemData.name.c_str(), &itemData.shown);
                    if (itemData.shown != wasShown)
                    {
                        itemData.shown ? itemData.item->OnShown() : itemData.item->OnHidden();
                    }
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar(1);
        }

        ImGui::End();
        ImGui::PopStyleVar(2);

        Size2f surfaceSize = window->GetSize();

        float32 scale = window->GetDPI() / 200.f;
        float32 buttonSide = 50.f * scale;

        ImGui::SetNextWindowPos(ImVec2(surfaceSize.dx - buttonSide - 20.f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        if (ImGui::Begin("DebugOverlayScaleButtons", nullptr, windowFlags))
        {
            ImGui::SetCursorPosY(20.f);

            PushColor(0.3f);
            if (ImGui::Button("+", { buttonSide, buttonSide }) && ImGui::GetIO().FontGlobalScale <= 5.0f)
            {
                ImGui::GetIO().FontGlobalScale += 0.1f;
                ImGui::GetStyle().ScrollbarSize += 2.f;
            }
            PopColor();

            ImGui::SetCursorPosY(buttonSide + 40.f);

            PushColor(0.6f);
            if (ImGui::Button("=", { buttonSide, buttonSide }))
            {
                ImGui::GetIO().FontGlobalScale = 1.f;
                ImGui::GetStyle().ScrollbarSize = 15.f; // TODO: think about mult. style imgui
            }
            PopColor();

            ImGui::SetCursorPosY(2.f * buttonSide + 60.f);

            PushColor(1.f);
            if (ImGui::Button("-", { buttonSide, buttonSide }) && ImGui::GetIO().FontGlobalScale >= 0.5f)
            {
                ImGui::GetIO().FontGlobalScale -= 0.1f;
                ImGui::GetStyle().ScrollbarSize -= 2.f;
            }
            PopColor();
        }
        ImGui::End();
        ImGui::PopStyleVar(1);
        ImGui::PopStyleColor(1);

        for (ItemData& itemData : items)
        {
            if (itemData.shown)
            {
                itemData.item->Draw();
            }
        }
    }
}
}
