#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QWidget>
#include <QScopedPointer>
#include <QPointer>
#include <QMap>


namespace Ui {class ColorWidget;};


class AbstractColorPalette;


class ColorWidget
    : public QWidget
{
    Q_OBJECT

private:
    typedef QMap< QString, QPointer<AbstractColorPalette> > PaletteMap;

public:
    explicit ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

    void AddPalette( const QString& name, AbstractColorPalette *pal );

private slots:
    void onPaletteType();

private:
    QScopedPointer<Ui::ColorWidget> ui;
    PaletteMap paletteMap;
};


#endif // COLORWIDGET_H
