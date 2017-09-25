#pragma once

#include <Math/Vector.h>

namespace DAVA
{
class UIGeometricData;
namespace TArc
{
class ContextAccessor;
}
}

namespace Painting
{
class Painter;
}

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
    DAVA::uint32 order = 0;
    Painting::Painter* painter = nullptr;
};

//lines have different behavior, so use base class to draw them
class DistanceLine
{
public:
    DistanceLine(const LineParams& params);
    virtual ~DistanceLine();

    virtual void Draw() = 0;

protected:
    LineParams params;
};

class SolidLine : public DistanceLine
{
public:
    SolidLine(const LineParams& params);

private:
    void Draw() override;

    void DrawEndLine();
    void DrawSolidLine();
    void DrawLineText();
};

class DotLine : public DistanceLine
{
public:
    DotLine(const LineParams& params);

private:
    void Draw() override;
};
