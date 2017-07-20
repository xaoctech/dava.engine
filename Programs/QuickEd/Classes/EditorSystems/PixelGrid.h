#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "UI/Preview/Data/CanvasDataAdapter.h"

#include <Base/Introspection.h>
#include <Math/Vector.h>
#include <Math/Color.h>

namespace DAVA
{
class Any;
}

class PixelGridPreferences : public DAVA::InspBase
{
public:
    PixelGridPreferences();
    ~PixelGridPreferences() override;

    DAVA::Color GetGridColor() const;
    void SetGridColor(const DAVA::Color& color);

    bool IsVisible() const;
    void SetVisible(bool visible);

    DAVA::float32 GetScaleToDisplay() const;
    void SetScaleToDisplay(DAVA::float32 scale);

    DAVA::Signal<> settingsChanged;

    INTROSPECTION(PixelGridPreferences,
                  PROPERTY("gridColor", "Pixel grid/color", GetGridColor, SetGridColor, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("isVisible", "Pixel grid/visible", IsVisible, SetVisible, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  PROPERTY("scaleToDisplay", "Pixel grid/display on scale", GetScaleToDisplay, SetScaleToDisplay, DAVA::I_SAVE | DAVA::I_VIEW | DAVA::I_EDIT | DAVA::I_PREFERENCE)
                  )

private:
    DAVA::Color gridColor;
    bool isVisible;
    DAVA::float32 scaleToDisaply;
};

class PixelGrid : public BaseEditorSystem
{
public:
    PixelGrid(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    ~PixelGrid() override;

private:
    void InitControls();

    void OnVisualSettingsChanged(const DAVA::Any&);

    bool CanShowGrid() const;
    void UpdateGrid();

    PixelGridPreferences preferences;

    DAVA::RefPtr<DAVA::UIControl> vLinesContainer;
    DAVA::RefPtr<DAVA::UIControl> hLinesContainer;

    CanvasDataAdapter canvasDataAdapter;
};
