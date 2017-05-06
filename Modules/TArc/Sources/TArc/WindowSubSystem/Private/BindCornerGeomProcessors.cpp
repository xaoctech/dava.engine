#include "TArc/WindowSubSystem/BindCornerGeomProcessors.h"

namespace DAVA
{
namespace TArc
{
BindCornerGeomProcessor::BindCornerGeomProcessor(Corner corner_, const QPoint& offset_)
    : corner(corner_)
    , offset(offset_)
{
}

QRect BindCornerGeomProcessor::GetWidgetGeometry(QWidget* parent, QWidget* content) const
{
    QRect parentGeometry = parent->geometry();
    QRect result;
    QSize contentSize = content->sizeHint();

    switch (corner)
    {
    case DAVA::TArc::BindCornerGeomProcessor::Corner::TopLeft:
    {
        result.setTopLeft(offset);
    }
    break;
    case DAVA::TArc::BindCornerGeomProcessor::Corner::TopRight:
    {
        QPoint corner(parentGeometry.size().width(), 0);
        corner.rx() -= (offset.x() + contentSize.width());
        corner.ry() += offset.y();

        result.setTopLeft(corner);
    }
    break;
    case DAVA::TArc::BindCornerGeomProcessor::Corner::BottomLeft:
    {
        QPoint corner(0, parentGeometry.size().height());
        corner.rx() += offset.x();
        corner.ry() -= (offset.y() + contentSize.height());

        result.setTopLeft(corner);
    }
    break;
    case DAVA::TArc::BindCornerGeomProcessor::Corner::BottomRight:
    {
        QSize parentSize = parentGeometry.size();
        QPoint corner(parentSize.width(), parentSize.height());
        corner.rx() -= (offset.x() + contentSize.width());
        corner.ry() -= (offset.y() + contentSize.height());

        result.setTopLeft(corner);
    }
    break;
    default:
        break;
    }

    result.setSize(contentSize);
    return result;
}

} // namespace TArc
} // namespace DAVA