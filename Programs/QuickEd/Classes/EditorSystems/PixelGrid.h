#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "UI/Preview/Data/CanvasDataAdapter.h"

#include <TArc/DataProcessing/SettingsNode.h>

#include <Math/Vector.h>
#include <Math/Color.h>

namespace DAVA
{
class Any;
}

class PixelGridPreferences : public DAVA::TArc::SettingsNode
{
public:
    DAVA::Color gridColor = DAVA::Color(0.925f, 0.925f, 0.925f, 0.5f);
    bool isVisible = true;
    DAVA::float32 scaleToDisplay = 800.0f;

    DAVA_VIRTUAL_REFLECTION(PixelGridPreferences, DAVA::TArc::SettingsNode);
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

    DAVA::RefPtr<DAVA::UIControl> vLinesContainer;
    DAVA::RefPtr<DAVA::UIControl> hLinesContainer;

    CanvasDataAdapter canvasDataAdapter;
};
