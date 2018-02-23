#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseLandscapeTool.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/Controls/ComboBox.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Qt/QtString.h>

#include <Reflection/Reflection.h>

#include <QGridLayout>
#include <QLabel>

namespace DAVA
{
class BrushWidgetBuilder
{
public:
    BrushWidgetBuilder(const BaseLandscapeTool::WidgetParams& params, const Reflection& model);

    template <typename T>
    void RegisterParam(const String& title, ControlDescriptorBuilder<typename T::Fields>& fields);

    void AddRow(const String& title, ControlProxy* control);

    QWidget* GetWidget();

private:
    BaseLandscapeTool::WidgetParams params;
    Reflection model;

    Widget* widget = nullptr;
    QGridLayout* mainLayout = nullptr;
};

template <typename T>
void BrushWidgetBuilder::RegisterParam(const String& title, ControlDescriptorBuilder<typename T::Fields>& fields)
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

    typename T::Params p(params.accessor, params.ui, params.wndKey);
    p.fields = fields;
    T* control = new T(p, params.processor, model, parent);
    control->ForceUpdate();
    widget->HandleControl(control);

    mainLayout->addWidget(control->ToWidgetCast(), nextRow, column, 1, columnSpan);
}
} // namespace DAVA
