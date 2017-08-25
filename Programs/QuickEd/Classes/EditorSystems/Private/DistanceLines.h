#pragma once

#include <Math/Vector.h>
#include <Render/2D/Font.h>

namespace DAVA
{
class UIControl;
class UIGeometricData;
namespace TArc
{
class ContextAccessor;
}
}

//lines have different behavior, so use base class to draw them
class DistanceLine
{
public:
    virtual void Draw(DAVA::UIControl* canvas) = 0;
};

class SolidLine : public DistanceLine
{
public:
    struct SolidLineParams
    {
        SolidLineParams(const DAVA::UIGeometricData& gd);

        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::Vector2 startPoint;
        DAVA::Vector2 endPoint;
        DAVA::RefPtr<DAVA::Font> font;
        const DAVA::UIGeometricData& gd;
        DAVA::eAlign direction;
    };

    SolidLine(const SolidLineParams& params);

private:
    void Draw(DAVA::UIControl* canvas) override;

    void DrawEndLine(DAVA::UIControl* canvas);
    void DrawSolidLine(DAVA::UIControl* canvas);
    void DrawLineText(DAVA::UIControl* canvas);

    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::Vector2 startPoint;
    DAVA::Vector2 endPoint;
    DAVA::RefPtr<DAVA::Font> font;
    const DAVA::UIGeometricData& parentGd;
    DAVA::eAlign direction;
};

class DotLine : public DistanceLine
{
public:
    struct DotLineParams
    {
        DotLineParams(const DAVA::UIGeometricData& gd);
        DAVA::TArc::ContextAccessor* accessor = nullptr;
        DAVA::Vector2 startPoint;
        DAVA::Vector2 endPoint;
        const DAVA::UIGeometricData& gd;
    };

    DotLine(const DotLineParams& params);

private:
    void Draw(DAVA::UIControl* canvas) override;
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::Vector2 startPoint;
    DAVA::Vector2 endPoint;
    const DAVA::UIGeometricData& parentGd;
};
