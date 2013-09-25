/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "BaseAddEntityDialog.h"
#include "ui_BaseAddEntityDialog.h"
#include "Qt/Main/QtUtils.h"
#include <QSizePolicy>
#include <Qlabel>
#include "QScrollBar.h"

#define PROPERTY_EDITOR_HEIGHT 300

BaseAddEntityDialog::BaseAddEntityDialog(QWidget* parent)
:	QDialog(parent),
	entity(NULL),
	ui(new Ui::BaseAddEntityDialog)
{
	ui->setupUi(this);
	setAcceptDrops(false);
	setWindowModality(Qt::NonModal);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);	
	setAttribute( Qt::WA_MacAlwaysShowToolWindow);// on top of all applications

	ui->scrollArea->setVisible(false);
	ui->propEditor->setMinimumHeight(PROPERTY_EDITOR_HEIGHT);
}

BaseAddEntityDialog::~BaseAddEntityDialog()
{
	delete ui;
	SafeRelease(entity);
	for(DAVA::Map<QWidget*, QWidget*>::iterator it = additionalWidgetMap.begin(); it != additionalWidgetMap.end(); ++it)
	{
		delete it->second;
	}
	additionalWidgetMap.clear();
}

void BaseAddEntityDialog::PerformResize()
{
	PropertyEditor* pEditor = ui->propEditor;
	pEditor->expandAll();
	this->adjustSize();
	ui->scrollArea->setMaximumHeight(ui->scrollArea->sizeHint().height());
	setMinimumHeight(sizeHint().height());
	
	bool isEditorFullyDisplayed = pEditor->verticalScrollBar()->maximum() == 0;
	QScrollArea* area = ui->scrollArea;
	bool isScrollAreaFullyDisplayed = area->isVisible() ?
	area->verticalScrollBar()->maximum() == 0 : true;
	int maxLimit = std::numeric_limits<int>::max();
	int maxHeight = isEditorFullyDisplayed? PROPERTY_EDITOR_HEIGHT : maxLimit;
	
	ui->propEditor->setMaximumHeight(maxHeight);
	maxHeight = isScrollAreaFullyDisplayed ? area->sizeHint().height() : maxLimit;
	area->setMaximumHeight(maxHeight);
	
	this->adjustSize();
	maxHeight = (isEditorFullyDisplayed && isScrollAreaFullyDisplayed) ? sizeHint().height() : 800;
	setMaximumHeight(maxHeight);
}

void BaseAddEntityDialog::hideEvent ( QHideEvent * event )
{
	QDialog::hideEvent(event);
	if(entity)
	{
		ui->propEditor->SetNode(NULL);
	}
}

void BaseAddEntityDialog::SetEntity(DAVA::Entity* _entity)
{
	SafeRelease(entity);
	
	entity = _entity;
	SafeRetain(entity);
	if(entity)
	{
		setWindowTitle(QString("Add ") + QString(entity->GetName().c_str()));
		PropertyEditor* pEditor = ui->propEditor;	

		pEditor->SetNode(entity);
		pEditor->expandAll();
		PerformResize();
	}
}

void BaseAddEntityDialog::AddControlToUserContainer(QWidget* widget)
{
	ui->userContentLayout->addWidget(widget);
	ui->scrollArea->setVisible(ui->userContentLayout->rowCount()>0);
}


void BaseAddEntityDialog::AddControlToUserContainer(QWidget* widget, const DAVA::String& labelString)
{
	QLabel* label = new QLabel(labelString.c_str(),this);
	int rowCount = ui->userContentLayout->rowCount();
	ui->userContentLayout->addWidget(label, rowCount, 0);
	ui->userContentLayout->addWidget(widget, rowCount, 1);
	additionalWidgetMap[widget] = label;
	ui->scrollArea->setVisible(ui->userContentLayout->rowCount()>0);
}

void BaseAddEntityDialog::RemoveControlFromUserContainer(QWidget* widget)
{
	ui->userContentLayout->removeWidget(widget);
	if(additionalWidgetMap.find(widget) != additionalWidgetMap.end())
	{
		QWidget* additionalWidget = additionalWidgetMap[widget];
		additionalWidgetMap.erase(additionalWidgetMap.find(widget));
		delete additionalWidget;
	}
}

void BaseAddEntityDialog::RemoveAllControlsFromUserContainer()
{
	QLayout* container = ui->userContentLayout;
	while (QLayoutItem* item = container->takeAt(0))
	{
		if (QWidget* widget = item->widget())
		{
			container->removeWidget( widget);
			if(additionalWidgetMap.find(widget) != additionalWidgetMap.end())
			{
				QWidget* additionalWidget = additionalWidgetMap[widget];
				additionalWidgetMap.erase(additionalWidgetMap.find(widget));
				delete additionalWidget;
			}
		}
	}
}

void BaseAddEntityDialog::GetIncludedControls(QList<QWidget*>& includedWidgets)
{
	QLayout* container = ui->userContentLayout;
	for (int i = 0; i < container->count(); ++i)
	{
		QWidget* child = container->itemAt(i)->widget();
		if(child)
		{
			includedWidgets.append(child);
		}
	}
}

