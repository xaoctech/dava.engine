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

//factory to create distance lines between two controls
class DistanceLinesFactory
{
public:
    struct Params
    {
        Params(DAVA::UIControl* selectedControl, DAVA::UIControl* highlightedControl);
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::Rect selectedRect;
        DAVA::Rect highlightedRect;
        DAVA::UIGeometricData parentGd;
        Painting::Painter* painter = nullptr;
    };

    DAVA::Vector<std::unique_ptr<DistanceLine>> CreateLines(const Params& params) const;

private:
    LineParams CreateLineParams(const Params& params, const DAVA::Vector2& startPoint, const DAVA::Vector2& endPos, DAVA::eAlign direction) const;

    void SurroundWithDotLines(const Params& params, DAVA::Vector2::eAxis axis, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const;
    template <typename T>
    void AddLine(const Params& params, DAVA::Vector2::eAxis axis, const DAVA::Vector2& startPos, const DAVA::Vector2& endPos, DAVA::Vector<std::unique_ptr<DistanceLine>>& lines) const;
};
