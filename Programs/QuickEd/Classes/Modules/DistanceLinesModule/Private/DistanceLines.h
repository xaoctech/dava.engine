#pragma once

#include <Math/Vector.h>

namespace DAVA
{
class UIControl;
class UIGeometricData;
namespace TArc
{
class ContextAccessor;
}
}

class Painter;

struct LineParams
{
    LineParams(const DAVA::UIGeometricData& gd);
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::Vector2 startPoint;
    DAVA::Vector2 endPoint;
    const DAVA::UIGeometricData& gd;
    DAVA::Vector2::eAxis axis;
    DAVA::Vector2::eAxis oppositeAxis;
    DAVA::eAlign direction;
    Painter* painter = nullptr;
};

//lines have different behavior, so use base class to draw them
class DistanceLine
{
public:
    DistanceLine(const LineParams& params);
    virtual void Draw(DAVA::UIControl* canvas) = 0;

protected:
    LineParams params;
};

class SolidLine : public DistanceLine
{
public:
    SolidLine(const LineParams& params);

private:
    void Draw(DAVA::UIControl* canvas) override;

    void DrawEndLine(DAVA::UIControl* canvas);
    void DrawSolidLine(DAVA::UIControl* canvas);
    void DrawLineText(DAVA::UIControl* canvas);
};

class DotLine : public DistanceLine
{
public:
    DotLine(const LineParams& params);

private:
    void Draw(DAVA::UIControl* canvas) override;
};
