#include "Tools/QtWaitDialog/QtWaitDialog.h"


QtWaitDialog::QtWaitDialog(QWidget *parent /*= 0*/)
	: QProgressDialog(parent, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	waitLabel = new QLabel(this);
	waitLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	setLabel(waitLabel);

	waitButton = new QPushButton("Cancel", this);
	setCancelButton(waitButton);

	waitBar = new QProgressBar(this);
	setBar(waitBar);

	QObject::connect(this, SIGNAL(canceled()), this, SLOT(WaitCanceled()));

	setMinimumSize(400, 150);
	setWindowModified(Qt::WindowModal);
}

QtWaitDialog::~QtWaitDialog()
{

}

void QtWaitDialog::Exec(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	setWindowTitle(title);
	setLabelText(message);

	waitButton->setEnabled(hasCancel);
	waitBar->setVisible(hasWaitbar);

	exec();
}

void QtWaitDialog::Show(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	setWindowTitle(title);
	setLabelText(message);

	waitButton->setEnabled(hasCancel);
	waitBar->setVisible(hasWaitbar);

	show();
}

void QtWaitDialog::Reset()
{
	reset();
	close();
}

void QtWaitDialog::SetMessage( const QString &message )
{
	setLabelText(message);
}

void QtWaitDialog::SetRange(int min, int max)
{
	setRange(0, 100);
}

void QtWaitDialog::SetRangeMin(int min)
{
	setMinimum(min);
}

void QtWaitDialog::SetRangeMax(int max)
{
	setMaximum(max);
}

void QtWaitDialog::SetValue(int value)
{
	waitBar->setVisible(true);
	setValue(value);
}

void QtWaitDialog::WaitCanceled()
{
	waitButton->setEnabled(false);
}
