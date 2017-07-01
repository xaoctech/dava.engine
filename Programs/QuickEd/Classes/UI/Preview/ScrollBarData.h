#pragma once

#include <TArc/DataProcessing/DataWrapper.h>

#include <Base/BaseTypes.h>
#include <Reflection/Reflection.h>
#include <Math/Vector.h>

#include <Qt>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
}

//Qt scrollbar can not be in invalid state and uses int for values and range

class ScrollBarData
{
public:
    static DAVA::FastName positionPropertyName;
    static DAVA::FastName minPosPropertyName;
    static DAVA::FastName maxPosPropertyName;
    static DAVA::FastName pageStepPropertyName;
    static DAVA::FastName enabledPropertyName;
    static DAVA::FastName visiblePropertyName;
    static DAVA::FastName orientationPropertyName;

    ScrollBarData(DAVA::Vector2::eAxis orientation, DAVA::TArc::ContextAccessor* accessor);

private:
    int GetPosition() const;
    void SetPosition(int position);

    int GetMinPos() const;
    int GetMaxPos() const;

    int GetPageStep() const;

    bool IsEnabled() const;

    bool IsVisible() const;

    int GetRange() const;

    Qt::Orientation GetOrientation() const;

    const DAVA::Vector2::eAxis orientation;

    DAVA::TArc::DataWrapper editorCanvasDataWrapper;
    DAVA::TArc::DataWrapper centralWidgetDataWrapper;

    DAVA_REFLECTION(ScrollBarData);
};
