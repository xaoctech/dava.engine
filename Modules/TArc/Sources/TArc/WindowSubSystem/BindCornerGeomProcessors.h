#pragma once

#include "TArc/WindowSubSystem/UI.h"

#include <QRect>

namespace DAVA
{
namespace TArc
{
class BindCornerGeomProcessor : public IGeometryProcessor
{
public:
    enum class Corner
    {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    BindCornerGeomProcessor(Corner corner, const QPoint& offset);

    QRect GetWidgetGeometry(QWidget* parent, QWidget* content) const override;

private:
    Corner corner;
    QPoint offset;
};
} // namespace TArc
} // namespace DAVA