#include <QApplication>
#include "Tools/QtWaitDialog/QtWaitDialog.h"
#include "ui_waitdialog.h"

QtWaitDialog::QtWaitDialog(QWidget *parent /*= 0*/)
	: QDialog(parent, Qt::Dialog | Qt::Tool | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
	, ui(new Ui::QtWaitDialog)
{
	ui->setupUi(this);

	QPalette pal = palette();
	pal.setColor(QPalette::Base, Qt::transparent);
	ui->waitLabel->setPalette(pal);

	QObject::connect(ui->waitButton, SIGNAL(pressed()), this, SLOT(CancelPressed()));
	QObject::connect(this, SIGNAL(canceled()), this, SLOT(WaitCanceled()));

	setMinimumSize(400, 150);
	setMaximumWidth(400);
	setWindowModality(Qt::WindowModal);
}

QtWaitDialog::~QtWaitDialog()
{

}

void QtWaitDialog::Exec(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	setWindowTitle(title);
	SetMessage(message);

	ui->waitButton->setEnabled(hasCancel);
	ui->waitBar->setVisible(hasWaitbar);

	exec();
}

void QtWaitDialog::Show(const QString &title, const QString &message, bool hasWaitbar, bool hasCancel)
{
	setWindowTitle(title);
	SetMessage(message);

	ui->waitButton->setEnabled(hasCancel);
	ui->waitBar->setVisible(hasWaitbar);

	show();

	QApplication::processEvents();
}

void QtWaitDialog::Reset()
{
	emit canceled();
	close();

	QApplication::processEvents();
}

void QtWaitDialog::SetMessage(const QString &message)
{
	ui->waitLabel->setPlainText(message);
	QApplication::processEvents();
}

void QtWaitDialog::SetRange(int min, int max)
{
	ui->waitBar->setRange(min, max);
	QApplication::processEvents();
}

void QtWaitDialog::SetRangeMin(int min)
{
	ui->waitBar->setMinimum(min);
	QApplication::processEvents();
}

void QtWaitDialog::SetRangeMax(int max)
{
	ui->waitBar->setMaximum(max);
	QApplication::processEvents();
}

void QtWaitDialog::SetValue(int value)
{
	ui->waitBar->setVisible(true);
	ui->waitBar->setValue(value);
	QApplication::processEvents();
}

void QtWaitDialog::CancelPressed()
{
	emit canceled();
}

void QtWaitDialog::WaitCanceled()
{
	ui->waitButton->setEnabled(false);
	QApplication::processEvents();
}
