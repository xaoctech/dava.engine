#ifndef VALUESLIDER_H
#define VALUESLIDER_H

#include <QWidget>
#include <QPointer>


class QLineEdit;
class MouseHelper;

class ValueSlider
    : public QWidget
{
    Q_OBJECT

public:
    explicit ValueSlider(QWidget *parent);
    ~ValueSlider();

protected:
    virtual void DrawBackground( QPainter *p ) const;
    virtual void DrawForeground( QPainter *p ) const;
    virtual QRect PosArea() const;

    void paintEvent( QPaintEvent* e ) override;
    void resizeEvent( QResizeEvent* e ) override;

    bool eventFilter( QObject* obj, QEvent* e ) override;

    bool IsEditorMode() const;

private slots:
    void OnMousePress( const QPoint& pos );
    void OnMouseMove( const QPoint& pos );
    void OnMouseRelease( const QPoint& pos );
    void OnMouseClick();

private:
    double minVal;
    double maxVal;
    double val;

    QPointer< MouseHelper > mouse;
    QPoint clickPos;
    double clickVal;

    QPointer<QLineEdit> editor;
};


#endif // VALUESLIDER_H
