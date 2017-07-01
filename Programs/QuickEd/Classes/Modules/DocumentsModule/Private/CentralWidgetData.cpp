#include "Modules/DocumentsModule/CentralWidgetData.h"

#include <QWidget>

DAVA::FastName CentralWidgetData::viewSizePropertyName{ "view size" };
DAVA::FastName CentralWidgetData::guidesPosPropertyName{ "guides position" };
DAVA::FastName CentralWidgetData::guidesSizePropertyName{ "guides size" };

DAVA_VIRTUAL_REFLECTION_IMPL(CentralWidgetData)
{
    DAVA::ReflectionRegistrator<CentralWidgetData>::Begin()
    .Field(viewSizePropertyName.c_str(), &CentralWidgetData::GetViewSize, nullptr)
    .Field(guidesPosPropertyName.c_str(), &CentralWidgetData::GetGuidesPos, nullptr)
    .Field(guidesSizePropertyName.c_str(), &CentralWidgetData::GetGuidesSize, nullptr)
    .End();
}

CentralWidgetData::CentralWidgetData(QWidget* renderWidget_, QWidget* hRulerWidget_, QWidget* vRulerWidget_)
    : renderWidget(renderWidget_)
    , hRulerWidget(hRulerWidget_)
    , vRulerWidget(vRulerWidget_)
{
}

DAVA::Vector2 CentralWidgetData::GetViewSize() const
{
    using namespace DAVA;

    QSize size = renderWidget->size();
    return Vector2(static_cast<float32>(size.width()),
                   static_cast<float32>(size.height()));
}

DAVA::Vector2 CentralWidgetData::GetGuidesPos() const
{
    using namespace DAVA;

    return Vector2(static_cast<float32>(vRulerWidget->pos().x()),
                   static_cast<float32>(hRulerWidget->pos().y()));
}

DAVA::Vector2 CentralWidgetData::GetGuidesSize() const
{
    using namespace DAVA;

    QPoint topRight = hRulerWidget->geometry().topRight();
    QPoint bottomLeft = vRulerWidget->geometry().bottomLeft();

    return Vector2(static_cast<float32>(topRight.x() - bottomLeft.x()),
                   static_cast<float32>(topRight.y() - bottomLeft.y()));
}
