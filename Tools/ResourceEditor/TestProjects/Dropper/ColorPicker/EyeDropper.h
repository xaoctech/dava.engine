#ifndef EYEDROPPER_H
#define EYEDROPPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>


class MouseHelper;

class EyeDropper
    : public QObject
{
    Q_OBJECT

    typedef QList< QPointer< QWidget > > ShadesList;

signals:
    void clicked( const QColor& color );
    void moved( const QColor& color );

public:
    explicit EyeDropper(QObject *parent = NULL);
    ~EyeDropper();

public slots:
    void Exec();

private slots:
    void OnMouseMove();
    void OnClicked();

private:
    bool eventFilter( QObject* obj, QEvent* e );
    void CreateShade();
    QColor GetPixel() const;

    QPointer<QWidget> shade;
    QPointer<MouseHelper> mouse;
};


#endif // EYEDROPPER_H
