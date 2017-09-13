#pragma once

#include "Classes/Modules/DistanceLinesModule/Private/DistanceLines.h"

#include <Base/BaseTypes.h>
#include <Math/Rect.h>
#include <UI/UIGeometricData.h>

namespace DAVA
{
namespace TArc
{
class ContextAccessor;
}
}

class LinesFactory
{
public:
    virtual DAVA::Vector<std::unique_ptr<DistanceLine>> CreateLines() const = 0;
};

//factory to create distance lines between two controls
class ControlsLinesFactory : public LinesFactory
{
public:
    struct ControlLinesFactoryParams
    {
        ControlLinesFactoryParams(DAVA::UIControl* selectedControl, DAVA::UIControl* highlightedControl);
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::Rect selectedRect;
        DAVA::Rect highlightedRect;
        DAVA::UIGeometricData parentGd;
        Painter* painter;
    };

    ControlsLinesFactory(const ControlLinesFactoryParams& params);

private:
    DAVA::Vector<std::unique_ptr<DistanceLine>> CreateLines() const override;

    LineParams CreateLineParams(const DAVA::Vector2& startPoint, const DAVA::Vector2& endPos, DAVA::eAlign direction) const;

    void SurroundWithDotLines(DAVA::Vector2::eAxis axis, const DAVA::Rect& rect, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const;
    template <typename T>
    void AddLine(DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const;

    ControlLinesFactoryParams params;
};
