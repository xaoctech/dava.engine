#include "QResourceEditorProgressDialog.h"
#include <qtimer.h>
#include <qprogressbar.h>

#define TIME_INTERVAL_FOR_1_PARCENT 10

QResourceEditorProgressDialog::QResourceEditorProgressDialog( QWidget * parent, Qt::WindowFlags f, bool _isCycled) : QProgressDialog(parent, f)
{
	isCycled = _isCycled;
	timeIntervalForParcent = TIME_INTERVAL_FOR_1_PARCENT;

	if(isCycled)
	{
		//add custom bar to avoid parcents displaying
		QProgressBar *bar = new QProgressBar(this);
		bar->setTextVisible(false);
		this->setBar(bar);
	}
}

void	QResourceEditorProgressDialog::showEvent ( QShowEvent * e )
{
	QProgressDialog::showEvent(e);
	if(isCycled)
	{
		QTimer::singleShot(TIME_INTERVAL_FOR_1_PARCENT, this, SLOT(UpdateProgress()));
	}
}

void	QResourceEditorProgressDialog::UpdateProgress()
{
	if( (!this->isVisible()) || (isCycled == false) )
	{
		return;
	}

	int newValue = this->value() + 1;
	if(newValue >= maximum())
	{
		newValue = 0;
	}
	
	this->setValue(newValue);
	QTimer::singleShot(timeIntervalForParcent, this, SLOT(UpdateProgress()));
}
