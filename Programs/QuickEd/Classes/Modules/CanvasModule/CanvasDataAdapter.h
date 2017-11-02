#pragma once

#include <TArc/DataProcessing/DataWrapper.h>

#include <Math/Vector.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
class ContextAccessor;
}

class CanvasData;
class CentralWidgetData;

class CanvasDataAdapter : public DAVA::ReflectionBase
{
public:
    explicit CanvasDataAdapter(DAVA::ContextAccessor* accessor);
    ~CanvasDataAdapter() override;

    static DAVA::FastName scalePropertyName;

    static DAVA::FastName positionPropertyName;
    static DAVA::FastName minimumPositionPropertyName;
    static DAVA::FastName maximumPositionPropertyName;

    //values displayed on screen. Used by rulers, guides and grid
    static DAVA::FastName startValuePropertyName;
    static DAVA::FastName lastValuePropertyName;

    //current screen position in abs coordinates
    static DAVA::FastName movableControlPositionPropertyName;

    DAVA::Vector2 GetPosition() const;
    void SetPosition(const DAVA::Vector2& pos);

    DAVA::Vector2 GetMinimumPosition() const;
    DAVA::Vector2 GetMaximumPosition() const;

    DAVA::Vector2 GetStartValue() const;
    DAVA::Vector2 GetLastValue() const;

    DAVA::float32 GetScale() const;
    void SetScale(DAVA::float32 scale);
    void SetScale(DAVA::float32 scale, const DAVA::Vector2& referencePoint);

    DAVA::Vector2 GetMovableControlPosition() const;

    DAVA::Vector2 GetViewSize() const;

    DAVA::Vector2 MapFromRootToScreen(const DAVA::Vector2& absValue) const;
    DAVA::Vector2 MapFromScreenToRoot(const DAVA::Vector2& position) const;

    DAVA::float32 MapFromRootToScreen(DAVA::float32 absValue, DAVA::Vector2::eAxis axis) const;
    DAVA::float32 MapFromScreenToRoot(DAVA::float32 position, DAVA::Vector2::eAxis axis) const;

private:
    DAVA::DataWrapper canvasDataWrapper;

    const CanvasData* GetCanvasData() const;

    DAVA::ContextAccessor* accessor = nullptr;

    DAVA_VIRTUAL_REFLECTION(CanvasDataAdapter, DAVA::ReflectionBase);
};
