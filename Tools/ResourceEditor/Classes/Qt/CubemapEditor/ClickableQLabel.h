#ifndef __QT_CLICKABLE_QLABEL_H__
#define __QT_CLICKABLE_QLABEL_H__

#include <QLabel.h>

class ClickableQLabel : public QLabel
{
	Q_OBJECT
	
public:
	
	ClickableQLabel(QWidget *parent = 0);
	~ClickableQLabel();
	
protected:
	
	void mousePressEvent(QMouseEvent *ev);
	
signals:

	void OnLabelClicked();

};

#endif /* defined(__QT_CLICKABLE_QLABEL_H__) */
