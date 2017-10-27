#include "Debug/DebugOverlay.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Engine/Window.h"
#include "Debug/Private/ImGui.h"
#include "Debug/DebugOverlayItem.h"

#include "Debug/Private/DebugOverlayItemEngineSettings.h"
#include "Debug/Private/DebugOverlayItemLogger.h"

namespace DAVA
{
    DebugOverlay::DebugOverlay()
    {
        GetPrimaryWindow()->update.Connect(this, &DebugOverlay::OnUpdate);

        static DebugOverlayItemEngineSettings es;
        static DebugOverlayItemLogger l;
        RegisterItem(&es);
        RegisterItem(&l);
    }

    DebugOverlay::~DebugOverlay()
    {
        Engine::Instance()->update.Disconnect(this);
    }

    void DebugOverlay::RegisterItem(DebugOverlayItem* overlayItem)
    {
        DVASSERT(overlayItem != nullptr);
        DVASSERT(std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) {return itemData.item == overlayItem; }) == items.end());

        ItemData itemData;
        itemData.item = overlayItem;
        itemData.name = overlayItem->GetName();
        itemData.enabled = false;

        items.push_back(itemData);
    }
    
    void DebugOverlay::UnregisterItem(DebugOverlayItem* overlayItem)
    {
        DVASSERT(overlayItem != nullptr);

        auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) {return itemData.item == overlayItem; });
        DVASSERT(iter != items.end());

        items.erase(iter);
    }

    void DebugOverlay::EnableItem(DebugOverlayItem* overlayItem)
    {
        DVASSERT(overlayItem != nullptr);

        auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) {return itemData.item == overlayItem; });
        DVASSERT(iter != items.end());

        iter->enabled = true;
    }

    void DebugOverlay::DisableItem(DebugOverlayItem* overlayItem)
    {
        DVASSERT(overlayItem != nullptr);

        auto iter = std::find_if(items.begin(), items.end(), [overlayItem](ItemData const& itemData) {return itemData.item == overlayItem; });
        DVASSERT(iter != items.end());

        iter->enabled = false;
    }

    void DebugOverlay::OnUpdate(Window* window, float32 timeDelta)
    {
        DVASSERT(ImGui::IsInitialized());
        
        if (ImGui::IsInitialized())
        {
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));
            if (ImGui::Begin("Debug views", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::Button("Overlay views"))
                {
                    ImGui::OpenPopup("Debug views");
                }
                
                ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetWindowSize().y));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5.0f, 5.0f));
                if (ImGui::BeginPopup("Debug views"))
                {
                    
                    for (ItemData& itemData : items)
                    {
                        ImGui::Checkbox(itemData.name.c_str(), &itemData.enabled);
                    }
                    
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar(1);

                
                ImGui::End();
            }
            ImGui::PopStyleVar(2);

            for (ItemData& itemData : items)
            {
                if (itemData.enabled)
                {
                    itemData.item->Draw();
                }
            }
        }
    }
}