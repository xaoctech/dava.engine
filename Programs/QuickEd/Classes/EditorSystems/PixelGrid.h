#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "UI/Preview/Data/CanvasDataAdapter.h"

#include <TArc/Utils/DirtyFrameUpdater.h>
#include <TArc/DataProcessing/DataWrapper.h>
#include <TArc/DataProcessing/DataListener.h>

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

class PixelGrid : public BaseEditorSystem, DAVA::TArc::DataListener
{
public:
    PixelGrid(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor);
    ~PixelGrid() override;

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitControls();

    void OnVisualSettingsChanged(const DAVA::Any&);

    bool CanShowGrid() const;
    void UpdateGrid();

    void OnDisplayStateChanged(EditorSystemsManager::eDisplayState currentState, EditorSystemsManager::eDisplayState previousState);

    PixelGridPreferences preferences;

    DAVA::RefPtr<DAVA::UIControl> vLinesContainer;
    DAVA::RefPtr<DAVA::UIControl> hLinesContainer;

    DirtyFrameUpdater updater;

    CanvasDataAdapter canvasDataAdapter;
    DAVA::TArc::DataWrapper canvasDataAdapterWrapper;
};
