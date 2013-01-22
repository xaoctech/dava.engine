#include "QtPropertyEditor/QtPropertyWidgets/QtColorLineEdit.h"

#include <QHBoxLayout>
#include <QStyle>
#include <QColorDialog>
#include <QRegExpValidator>

QtColorLineEdit::QtColorLineEdit(QWidget * parent)
	: QLineEdit(parent)
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

void QtColorLineEdit::SetColor(const QColor &color)
{
	curColor = color;
	setText(curColor.name());
}

QColor QtColorLineEdit::GetColor() const
{
	return curColor;
}

void QtColorLineEdit::ToolButtonClicked()
{
	QColorDialog *dlg = new QColorDialog(this);
	dlg->setCurrentColor(GetColor());
	dlg->setOption(QColorDialog::ShowAlphaChannel, true);

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
