#include "Vector2Edit.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>

Vector2Edit::Vector2Edit(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    QLabel * label1 = new QLabel(this);
    label1->setText("[");

    horizontalLayout->addWidget(label1);

    lineEditX = new QLineEdit(this);
    lineEditX->setObjectName(QString::fromUtf8("lineEditX"));
    lineEditX->setValidator( new QDoubleValidator(-9999.0f, 9999.0f, 4, this) );

    horizontalLayout->addWidget(lineEditX);

    QLabel * label2 = new QLabel(this);
    label2->setText(",");

    horizontalLayout->addWidget(label2);

    lineEditY = new QLineEdit(this);
    lineEditY->setObjectName(QString::fromUtf8("lineEditY"));
    lineEditY->setValidator( new QDoubleValidator(-9999.9999, 9999.9999, 4, this) );
    horizontalLayout->addWidget(lineEditY);

    QLabel * label3 = new QLabel(this);
    label3->setText("]");

    horizontalLayout->addWidget(label3);

    horizontalLayout->setStretch(1, 1);
    horizontalLayout->setStretch(3, 1);

    setAutoFillBackground(true);

    //setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(lineEditX);
}

Vector2Edit::~Vector2Edit()
{
}

QVector2D Vector2Edit::value() const
{
    return QVector2D(lineEditX->text().toFloat(), lineEditY->text().toFloat() );
}
void Vector2Edit::setValue( const QVector2D &newValue)
{
    lineEditX->setText(QString("%1").arg(newValue.x()));//::fromStdString(DAVA::Format("%g", newValue.x())));
    lineEditY->setText(QString("%1").arg(newValue.y()));//QString::fromStdString(DAVA::Format("%g", newValue.y())));
}

bool Vector2Edit::isModified() const
{
    return lineEditX->isModified() || lineEditY->isModified();
}

