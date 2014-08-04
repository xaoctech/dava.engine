#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QScopedPointer>
#include <QMap>

namespace Ui {class ColorPicker;};

class AbstractColorPicker;


class ColorPicker
    : public QWidget
{
    Q_OBJECT

private:
    typedef QMap< QString, AbstractColorPicker * > PickerMap;

public:
    explicit ColorPicker(QWidget *parent = 0);
    ~ColorPicker();

protected:
    void RegisterPicker( const QString& key, AbstractColorPicker *picker );

private:
    QScopedPointer<Ui::ColorPicker> ui;
    PickerMap pickers;
};

#endif // COLORPICKER_H
