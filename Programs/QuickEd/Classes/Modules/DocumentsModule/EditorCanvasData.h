#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Math/Vector.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
}

//private data of EditorCanvas
//used to keep widget state between tabs

class EditorCanvasData : public DAVA::TArc::DataNode
{
public:
    EditorCanvasData(DAVA::TArc::ContextAccessor* accessor);
    ~EditorCanvasData() override = default;

    //size of all controls without scale and margins
    static DAVA::FastName workAreaSizePropertyName;
    //size of all controls with scale and margins
    static DAVA::FastName canvasSizePropertyName;

    //canvas position
    //used by scrollBars and inputs in CanvasSystem
    static DAVA::FastName positionPropertyName;
    static DAVA::FastName minimumPositionPropertyName;
    static DAVA::FastName maximumPositionPropertyName;

    //root control position for set only
    //used for start value on rulers, guides and grid
    static DAVA::FastName rootPositionPropertyName;

    //values displayed on screen. Used by rulers, guides and grid
    static DAVA::FastName startValuePropertyName;
    static DAVA::FastName lastValuePropertyName;

    //canvas scale
    static DAVA::FastName scalePropertyName;
    //predefined scales list
    static DAVA::FastName predefinedScalesPropertyName;

    //reference point are set to keep visible some area. Usually it's a cursor position or screen center
    //later it can be selection area center
    //reference point must be set before changing scale
    //otherwise screen will be centralized
    static DAVA::FastName referencePointPropertyName;

    //current screen position in abs coordinates
    static DAVA::FastName movableControlPositionPropertyName;

    DAVA::Vector2 GetWorkAreaSize() const;
    DAVA::Vector2 GetCanvasSize() const;

    DAVA::Vector2 GetPosition() const;
    DAVA::Vector2 GetMinimumPosition() const;
    DAVA::Vector2 GetMaximumPosition() const;

    DAVA::Vector2 GetStartValue() const;
    DAVA::Vector2 GetLastValue() const;

    DAVA::float32 GetScale() const;
    const DAVA::Vector<DAVA::float32>& GetPredefinedScales() const;

    DAVA::Vector2 GetMovableControlPosition() const;

    DAVA::Vector2 GetViewSize() const;

    //helper functions for mouse wheel and shortcuts
    DAVA::float32 EditorCanvasData::GetNextScale(DAVA::int32 ticksCount) const;
    DAVA::float32 EditorCanvasData::GetPreviousScale(DAVA::int32 ticksCount) const;

private:
    void SetWorkAreaSize(const DAVA::Vector2& size);
    void SetPosition(const DAVA::Vector2& position);
    DAVA::Vector2 GetRootPosition() const;
    void SetRootPosition(const DAVA::Vector2& rootPosition);
    void SetScale(DAVA::float32 scale);

    DAVA::Vector2 workAreaSize;

    DAVA::Vector2 position;

    //root control position in a in abs coordinates hierarchy
    DAVA::Vector2 rootPosition;

    //work area scale
    DAVA::float32 scale = 1.0f;
    DAVA::Vector<DAVA::float32> predefinedScales;
    DAVA::Vector2 referencePoint;
    const DAVA::Vector2 invalidPoint = DAVA::Vector2(-1.0f, -1.0f);

    const DAVA::float32 margin = 50.0f;

    //when new screen is open it must be centralized. But opening a new screen takes more than one frame, because viewSize can be changed by the tabBar
    bool needCentralize = true;

    DAVA::TArc::ContextAccessor* accessor = nullptr;

    DAVA_VIRTUAL_REFLECTION(EditorCanvasData, DAVA::TArc::DataNode);
};
