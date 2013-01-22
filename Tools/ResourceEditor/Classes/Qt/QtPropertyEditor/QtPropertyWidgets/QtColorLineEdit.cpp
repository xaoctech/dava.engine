#include "QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

#include <QHBoxLayout>
#include <QEvent>
#include <QStyle>
#include <QColorDialog>
#include <QRegExpValidator>
#include <QAbstractScrollArea>

QtColorLineEdit::QtColorLineEdit(QWidget * parent)
	: QLineEdit(parent)
	, deleg(NULL)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	toolButton = new QToolButton(this);
	toolButton->setText("...");

	QRegExpValidator *validator = new QRegExpValidator();
	validator->setRegExp(QRegExp("#[A-F0-9]{6}", Qt::CaseInsensitive));

	setValidator(validator);
	//setInputMask("\\#NNNNNN");

	setLayout(layout);
	layout->setContentsMargins(1, 1, 1, 1);
	layout->addStretch();
	layout->addWidget(toolButton);

	QObject::connect(toolButton, SIGNAL(clicked()), this, SLOT(ToolButtonClicked()));
	QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(EditFinished()));
}

QtColorLineEdit::~QtColorLineEdit()
{
	printf("111\n");
}

void QtColorLineEdit::SetColor(const QColor &color)
{
	curColor = color;
	setText(curColor.name());
}

QColor QtColorLineEdit::GetColor() const
{
	return curColor;
}

void QtColorLineEdit::SetItemDelegatePtr(const QAbstractItemDelegate* _deleg)
{
	deleg = _deleg;
}

void QtColorLineEdit::ToolButtonClicked()
{
	QColorDialog *dlg = new QColorDialog(this);
	dlg->setCurrentColor(GetColor());
	dlg->setOption(QColorDialog::ShowAlphaChannel, true);

	removeEventFilter((QObject *) deleg);
	if(QDialog::Accepted == dlg->exec())
	{
		SetColor(dlg->selectedColor());
	}

	delete dlg;
	close();
}

void QtColorLineEdit::EditFinished()
{
	curColor = QColor(text());
}
