#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <QScopedPointer>
#include <QMap>
#include <QEventLoop>
#include <QPointer>

namespace Ui
{
    class ColorPicker;
};

#include "AbstractColorPicker.h"


class EyeDropper;
class ColorPickerRGBAM;
class QtPosSaver;

class ColorPicker
    : public AbstractColorPicker
{
    Q_OBJECT

private:
    typedef QMap< QString, QPointer<AbstractColorPicker> > PickerMap;

public:
    explicit ColorPicker(QWidget* parent = 0);
    ~ColorPicker();

    void Exec();

    double GetMultiplierValue() const;
    void SetMultiplierValue(double val);

protected:
    void RegisterPicker(const QString& key, AbstractColorPicker* picker);
    void RegisterColorSpace(const QString& key, AbstractColorPicker* picker);

    void SetColorInternal(const QColor& c) override;

private slots:
    void OnChanging(const QColor& c);
    void OnChanged(const QColor& c);
    void OnDropperChanged(const QColor& c);

    void OnDropper();

private:
    void UpdateControls(const QColor& c, AbstractColorPicker* source = NULL);
    void ConnectPicker(AbstractColorPicker* picker);
    void closeEvent(QCloseEvent* e);

    QScopedPointer<Ui::ColorPicker> ui;
    QPointer<EyeDropper> dropper;
    QPointer<ColorPickerRGBAM> rgbam;
    PickerMap pickers;
    PickerMap colorSpaces;
    QEventLoop modalLoop;
    QScopedPointer<QtPosSaver> posSaver;
};

#endif // COLORPICKER_H
