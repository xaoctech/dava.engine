#pragma once

#include "TArc/Controls/LineEdit.h"
#include <QWidget>

namespace DAVA
{
namespace TArc
{
class PopupLineEdit : public QWidget
{
public:
    PopupLineEdit(const LineEdit::Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    PopupLineEdit(const LineEdit::Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);
    ~PopupLineEdit();

    void Show(const QPoint& position);
    void Show(const QRect& geometry);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

    void SetupControl();

private:
    LineEdit* edit = nullptr;
};

} // namespace TArc
} // namespace DAVA