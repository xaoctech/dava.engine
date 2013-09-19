/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "SliderWidget.h"

#include <QLayout>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

SliderWidget::SliderWidget(QWidget* parent)
:	QWidget(parent)
,	isRangeChangingBlocked(false)
{
	InitUI();
	Init("", true, 10, 0, 0);

	ConnectToSignals();
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::InitUI()
{
	QVBoxLayout* layout = new QVBoxLayout();

	sliderValue = new QSlider();
	editValue = new QLineEdit();
	editMinValue = new QLineEdit();
	spinMaxValue = new QSpinBox();
	labelCaption = new QLabel();

	QHBoxLayout* captionLayout = new QHBoxLayout();
	QSpacerItem* captionSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Maximum);
	captionLayout->addWidget(labelCaption);
	captionLayout->addWidget(editValue);
	captionLayout->addSpacerItem(captionSpacer);

	QHBoxLayout* sliderLayout = new QHBoxLayout();
	sliderLayout->addWidget(editMinValue);
	sliderLayout->addWidget(sliderValue);
	sliderLayout->addWidget(spinMaxValue);

	layout->addLayout(captionLayout);
	layout->addLayout(sliderLayout);

	setLayout(layout);

	layout->setContentsMargins(0, 0, 0, 0);
	captionLayout->setContentsMargins(0, 0, 0, 0);
	sliderLayout->setContentsMargins(0, 0, 0, 0);
	sliderValue->setOrientation(Qt::Horizontal);
	sliderValue->setTickPosition(QSlider::TicksBothSides);
	editValue->setMaximumWidth(60);
	editValue->setInputMethodHints(Qt::ImhDigitsOnly);
	editValue->setMaxLength(3);
	editMinValue->setMaximumWidth(40);
	editMinValue->setInputMethodHints(Qt::ImhDigitsOnly);
	editMinValue->setMaxLength(4);
	spinMaxValue->setMaximumWidth(55);
	spinMaxValue->setInputMethodHints(Qt::ImhDigitsOnly);
	spinMaxValue->setRange(1, 999);
	spinMaxValue->setValue(1);
}

void SliderWidget::ConnectToSignals()
{
	connect(sliderValue, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)));
	connect(spinMaxValue, SIGNAL(valueChanged(int)), this, SLOT(RangeChanged(int)));
	connect(editValue, SIGNAL(editingFinished()), this, SLOT(EditValueChanged()));
}

void SliderWidget::Init(const QString caption, bool symmetric, int max, int min, int value)
{
	this->caption = caption;
	isSymmetric = symmetric;
	minValue = min;
	maxValue = max;
	currentValue = value;

	UpdateControls();
}

void SliderWidget::SetRangeMax(int max)
{
	if (max <= 0 || max < GetRangeMin())
	{
		return;
	}

	maxValue = max;
	if (IsSymmetric())
	{
		minValue = -max;
	}

	if (currentValue > maxValue)
	{
		SetValue(maxValue);
	}
	else if (currentValue < minValue)
	{
		SetValue(minValue);
	}

	UpdateControls();
}

int SliderWidget::GetRangeMax()
{
	return maxValue;
}

void SliderWidget::SetRangeMin(int min)
{
	if (IsSymmetric() || min > GetRangeMax())
	{
		return;
	}

	minValue = min;
	if (currentValue < min)
	{
		SetValue(min);
	}

	UpdateControls();
}

int SliderWidget::GetRangeMin()
{
	return minValue;
}

void SliderWidget::SetSymmetric(bool symmetric)
{
	isSymmetric = symmetric;

	if (IsSymmetric())
	{
	}

	UpdateControls();
}

bool SliderWidget::IsSymmetric()
{
	return isSymmetric;
}

void SliderWidget::SetValue(int value)
{
	if (value == currentValue)
	{
		return;
	}

	value = qMax(minValue, value);
	value = qMin(maxValue, value);

	currentValue = value;
	EmitValueChanged();
	UpdateControls();
}

int SliderWidget::GetValue()
{
	return currentValue;
}

void SliderWidget::SliderValueChanged(int newValue)
{
	SetValue(newValue);
}

void SliderWidget::RangeChanged(int newMaxValue)
{
	if (newMaxValue <= 0 || newMaxValue <= GetRangeMin())
	{
		return;
	}

	SetRangeMax(newMaxValue);
	if (newMaxValue < currentValue)
	{
		SetValue(newMaxValue);
	}
}

void SliderWidget::EmitValueChanged()
{
	emit ValueChanged(currentValue);
}

void SliderWidget::UpdateControls()
{
	bool blocked = signalsBlocked();
	blockSignals(true);

	editMinValue->setText(QString::number(GetRangeMin()));
	if (GetRangeMin() > 0)
	{
		spinMaxValue->setMinimum(GetRangeMin());
	}
	spinMaxValue->setValue(GetRangeMax());
	sliderValue->setRange(GetRangeMin(), GetRangeMax());
	sliderValue->setValue(GetValue());
	editValue->setText(QString::number(GetValue()));

	labelCaption->setText(caption);

	blockSignals(blocked);
}

void SliderWidget::SetRangeChangingBlocked(bool blocked)
{
	if (blocked == isRangeChangingBlocked)
	{
		return;
	}

	spinMaxValue->setReadOnly(blocked);
	spinMaxValue->setButtonSymbols(blocked ? QAbstractSpinBox::NoButtons : QAbstractSpinBox::UpDownArrows);

	isRangeChangingBlocked = blocked;
}

bool SliderWidget::IsRangeChangingBlocked()
{
	return isRangeChangingBlocked;
}

void SliderWidget::SetCaption(const QString &caption)
{
	this->caption = caption;
	labelCaption->setText(caption);
}

QString SliderWidget::GetCaption()
{
	return caption;
}

void SliderWidget::EditValueChanged()
{
	int newValue = editValue->text().toInt();

	SetValue(newValue);
}
