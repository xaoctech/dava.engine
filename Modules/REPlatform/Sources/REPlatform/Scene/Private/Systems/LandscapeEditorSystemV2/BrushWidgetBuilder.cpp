#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h"

namespace DAVA
{
BrushWidgetBuilder::BrushWidgetBuilder(const BaseLandscapeTool::WidgetParams& params_, const DAVA::Reflection& model_)
    : params(params_)
    , model(model_)
{
    widget = new Widget(params.parent);
    mainLayout = new QGridLayout();
    mainLayout->setMargin(1);
    mainLayout->setSpacing(2);
    widget->SetLayout(mainLayout);
}

void BrushWidgetBuilder::AddRow(const String& title, ControlProxy* control)
{
    int nextRow = mainLayout->rowCount();
    bool hasTitle = !title.empty();

    QWidget* parent = widget->ToWidgetCast();

    if (hasTitle == true)
    {
        mainLayout->addWidget(new QLabel(QString::fromStdString(title), parent), nextRow, 0);
    }

    int column = 1;
    int columnSpan = 1;
    if (hasTitle == false)
    {
        column = 0;
        columnSpan = 2;
    }

    widget->HandleControl(control);
    mainLayout->addWidget(control->ToWidgetCast(), nextRow, column, 1, columnSpan);
}

QWidget* BrushWidgetBuilder::GetWidget()
{
    widget->ForceUpdate();
    return widget->ToWidgetCast();
}
} // namespace DAVA
