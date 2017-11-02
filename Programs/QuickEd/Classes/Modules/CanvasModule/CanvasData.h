#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

#include <Math/Vector.h>

//private data of EditorCanvas
//used to keep widget state between tabs

class CanvasData : public DAVA::TArcDataNode
{
public:
    CanvasData();
    ~CanvasData() override = default;

    //size of all controls without scale and margins
    static DAVA::FastName workAreaSizePropertyName;
    //size of all controls with scale and margins
    static DAVA::FastName canvasSizePropertyName;

    //canvas position
    //used by scrollBars and inputs in CanvasSystem
    static DAVA::FastName positionPropertyName;

    //root control position for set only
    //used for start value on rulers, guides and grid
    static DAVA::FastName rootPositionPropertyName;

    //canvas scale
    static DAVA::FastName scalePropertyName;
    //predefined scales list
    static DAVA::FastName predefinedScalesPropertyName;

    //centralize control on view
    static DAVA::FastName needCentralizePropertyName;

    //reference point are set to keep visible some area. Usually it's a cursor position or screen center
    //later it can be selection area center
    //reference point must be set before changing scale
    //otherwise screen will be centralized
    static DAVA::FastName referencePointPropertyName;

    DAVA::Vector2 GetWorkAreaSize() const;
    DAVA::Vector2 GetCanvasSize() const;

    DAVA::Vector2 GetPosition() const;

    DAVA::float32 GetScale() const;
    const DAVA::Vector<DAVA::float32>& GetPredefinedScales() const;

    //helper functions for mouse wheel and shortcuts
    DAVA::float32 GetNextScale(DAVA::int32 ticksCount) const;
    DAVA::float32 GetPreviousScale(DAVA::int32 ticksCount) const;

    DAVA::Vector2 GetRootPosition() const;

    bool IsCentralizeRequired() const;
    DAVA::float32 GetMargin() const;

private:
    void SetWorkAreaSize(const DAVA::Vector2& size);
    void SetPosition(const DAVA::Vector2& position);
    void SetRootPosition(const DAVA::Vector2& rootPosition);
    void SetScale(DAVA::float32 scale);

    DAVA::Vector2 workAreaSize;

    DAVA::Vector2 position;

    //root control position in a in abs coordinates hierarchy
    DAVA::Vector2 rootPosition;

    //work area scale
    DAVA::float32 scale = 1.0f;
    DAVA::Vector<DAVA::float32> predefinedScales;

    const DAVA::float32 margin = 50.0f;

    bool needCentralize = false;

    DAVA_VIRTUAL_REFLECTION(CanvasData, DAVA::TArcDataNode);
};
