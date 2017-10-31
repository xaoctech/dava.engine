#include "Debug/DebugOverlay.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Engine/Window.h"
#include "Concurrency/Thread.h"
#include "Debug/Private/ImGui.h"
#include "Debug/DebugOverlayItem.h"

#include "Debug/Private/DebugOverlayItemEngineSettings.h"
#include "Debug/Private/DebugOverlayItemLogger.h"

namespace DAVA
{
DebugOverlay::DebugOverlay()
    : defaultItemEngineSettings{ new DebugOverlayItemEngineSettings }
    , defaultItemLogger{ new DebugOverlayItemLogger }
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

    items.push_back(itemData);
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
}

void DebugOverlay::UnregisterDefaultItems()
{
    UnregisterItem(defaultItemEngineSettings.get());
    UnregisterItem(defaultItemLogger.get());
}

void DebugOverlay::OnUpdate(Window* window, float32 timeDelta)
{
    DVASSERT(ImGui::IsInitialized());

    if (ImGui::IsInitialized())
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));
        uint32 windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize;
        if (ImGui::Begin("DebugOverlayWindow", nullptr, windowFlags))
        {
            if (ImGui::Button("Debug views"))
            {
                ImGui::OpenPopup("DebugViewsPopup");
            }

            ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetWindowSize().y));
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

            ImGui::End();
        }
        ImGui::PopStyleVar(2);

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