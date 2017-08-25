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
struct DistanceLine
{
    virtual void Draw(DAVA::UIControl* canvas) = 0;
};

struct SolidLine : public DistanceLine
{
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
