#pragma once

#include <TArc/DataProcessing/DataWrappersProcessor.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/WindowSubsystem/UI.h>

#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/Material/NMaterial.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
class Texture;
class NMaterial;
class ContextAccessor;
class UI;
class WindowKey;

class LandscapeEditorSystemV2;
class BrushInputController;
class BaseBrushApplicant;
class RECommandNotificationObject;
class BaseLandscapeTool : public ReflectionBase
{
public:
    struct SortKeyImpl
    {
        SortKeyImpl(int32 key)
            : sortKey(key)
        {
        }
        int32 sortKey = -1;
    };

    using SortKey = Meta<SortKeyImpl>;

    BaseLandscapeTool(LandscapeEditorSystemV2* system_)
        : system(system_)
    {
    }

    virtual ~BaseLandscapeTool() = default;

    struct ButtonInfo
    {
        ButtonInfo() = default;
        ButtonInfo(const QIcon& icon_, const QString& tooltip_)
            : icon(icon_)
            , tooltip(tooltip_)
        {
        }

        QIcon icon;
        QString tooltip;
    };

    virtual ButtonInfo GetButtonInfo() const = 0;

    virtual void Activate(const PropertiesItem& settings) = 0;
    virtual BrushInputController* GetInputController() const = 0;
    virtual BaseBrushApplicant* GetBrushApplicant() const = 0;

    virtual void Process(float32 /*delta*/)
    {
    }

    struct WidgetParams
    {
        WidgetParams(const WindowKey& key)
            : wndKey(key)
        {
        }
        ContextAccessor* accessor = nullptr;
        UI* ui = nullptr;
        WindowKey wndKey;
        DataWrappersProcessor* processor = nullptr;
        QWidget* parent = nullptr;
    };
    virtual QWidget* CreateEditorWidget(const WidgetParams& params) = 0;
    // Texture that will be drawn over landscape. Can be nullptr.
    virtual RefPtr<Texture> GetCustomCoverTexture() const
    {
        return RefPtr<Texture>();
    }
    // Texture with cursor shape. That texture will be drawn around cursor with size get from GetBrushSize.
    virtual RefPtr<Texture> GetCursorTexture() const
    {
        return RefPtr<Texture>();
    }

    virtual Color GetCursorColor() const
    {
        return Color(0.5f, 0.5f, 1.0f, 1.0f);
    }

    // Brush size in UV coordinates according to Landscape size
    virtual Vector2 GetBrushSize() const = 0;
    virtual float32 GetBrushRotation() const = 0; // [0.0f, 1.0f]
    virtual void Deactivate(PropertiesItem& settings) = 0;

    void SetAccessor(ContextAccessor* accessor)
    {
        this->accessor = accessor;
    }

protected:
    LandscapeEditorSystemV2* system = nullptr;
    ContextAccessor* accessor = nullptr;

private:
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(BaseLandscapeTool, ReflectionBase)
    {
        ReflectionRegistrator<BaseLandscapeTool>::Begin()
        .End();
    }
};
} // namespace DAVA
