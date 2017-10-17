#ifndef BUTTONSWIDGET_H
#define BUTTONSWIDGET_H

#include <QWidget>
#include <QString>

namespace Ui
{
class ButtonsWidget;
}

class ButtonsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ButtonsWidget(int rowNum, QWidget* parent = 0);
    ~ButtonsWidget();

    void setRemoveEnabled(bool enabled);

signals:
    void OnRun(int rowNum);
    void OnRemove(int rowNum);

public slots:
    void OnRunClicked();
    void OnRemoveClicked();

private:
    Ui::ButtonsWidget* ui;
    int rowNumber;
};

#endif // BUTTONSWIDGET_H
