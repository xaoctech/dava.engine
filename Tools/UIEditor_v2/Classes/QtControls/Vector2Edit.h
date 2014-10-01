#ifndef __VECTOR2EDIT_H__
#define __VECTOR2EDIT_H__

#include <QWidget>
#include <QVector2D>

class QLineEdit;

class Vector2Edit: public QWidget
{
    Q_OBJECT
        Q_PROPERTY(QVector2D Vector2D READ value WRITE setValue USER true);
public:
    explicit Vector2Edit(QWidget *parent = NULL);
    ~Vector2Edit();

    QVector2D value() const;
    void setValue(const QVector2D &newValue);

    bool isModified() const;
private:
    QLineEdit *lineEditX;
    QLineEdit *lineEditY;
};

#endif // __VECTOR2EDIT_H__
