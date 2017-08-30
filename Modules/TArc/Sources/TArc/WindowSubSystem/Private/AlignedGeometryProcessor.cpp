#include "TArc/WindowSubSystem/AlignedGeometryProcessor.h"

namespace DAVA
{
namespace TArc
{
DAVA::eAlign ExtractAlign(eAlign alignment, int32 alignMask, eAlign defaultAlign)
{
    eAlign result = static_cast<eAlign>(alignment & alignMask);
    if (result == 0)
    {
        return defaultAlign;
    }

    DVASSERT((result & (result - 1)) == 0);
    return result;
}

AlignedGeometryProcessor::AlignedGeometryProcessor(DAVA::eAlign alignment_, const QPoint& offset_)
    : alignment(alignment_)
    , offset(offset_)
{
}

QRect AlignedGeometryProcessor::GetWidgetGeometry(QWidget* parent, QWidget* content) const
{
    QRect parentGeometry = parent->geometry();
    QRect result;
    QSize contentSize = content->sizeHint();

    eAlign hAlign = ExtractAlign(alignment, ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER, ALIGN_LEFT);
    eAlign vAlign = ExtractAlign(alignment, ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER, ALIGN_TOP);
    QPoint topLeftPoint;

    switch (hAlign)
    {
    case DAVA::ALIGN_LEFT:
        topLeftPoint.rx() = offset.x();
        break;
    case DAVA::ALIGN_HCENTER:
        topLeftPoint.rx() = parentGeometry.center().x() - (contentSize.width() >> 1);
        break;
    case DAVA::ALIGN_RIGHT:
        topLeftPoint.rx() = parentGeometry.size().width() - offset.x() - contentSize.width();
        break;
    default:
        DVASSERT(false);
        break;
    }

    switch (vAlign)
    {
    case DAVA::ALIGN_TOP:
        topLeftPoint.ry() = offset.y();
        break;
    case DAVA::ALIGN_VCENTER:
        topLeftPoint.ry() = parentGeometry.center().y() - (contentSize.height() >> 1);
        break;
    case DAVA::ALIGN_BOTTOM:
        topLeftPoint.ry() = parentGeometry.size().height() - offset.y() - contentSize.height();
        break;
    default:
        DVASSERT(false);
        break;
    }

    result.setTopLeft(topLeftPoint);
    result.setSize(contentSize);
    return result;
}

} // namespace TArc
} // namespace DAVA