#include "CubemapEditor/ClickableQLabel.h"

#include <QMouseEvent>

ClickableQLabel::ClickableQLabel(QWidget *parent) : QLabel(parent)
{
	
}

ClickableQLabel::~ClickableQLabel()
{
	
}

void ClickableQLabel::mousePressEvent(QMouseEvent *ev)
{
	if(ev->button() == Qt::LeftButton)
	{
		emit OnLabelClicked();
	}
	
	QLabel::mousePressEvent(ev);
}
