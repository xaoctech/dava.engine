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
#include "Scene/System/BeastSystem.h"


#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataKeyedArchiveMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"
#include "Main/mainwindow.h"

#define PROPERTY_EDITOR_HEIGHT	300
#define QT_HEIGHT_LIMIT			16777215
#define WINDOW_HEIGHT_LIMIT		800

BaseAddEntityDialog::BaseAddEntityDialog(QWidget* parent, QDialogButtonBox::StandardButtons buttons)
:	QDialog(parent),
	entity(NULL),
	ui(new Ui::BaseAddEntityDialog)
{
	ui->setupUi(this);
	setAcceptDrops(false);
	setWindowModality(Qt::NonModal);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);	
	setAttribute( Qt::WA_MacAlwaysShowToolWindow);// on top of all applications

	//ui->scrollArea->setVisible(false);
	propEditor = NULL;
	
	InitPropertyEditor();

	ui->buttonBox->setStandardButtons(buttons);
}

BaseAddEntityDialog::~BaseAddEntityDialog()
{
	propEditor->RemovePropertyAll();
	disconnect( propEditor, SIGNAL(PropertyEdited(const QString &, QtPropertyData *)), this, SLOT(OnItemEdited(const QString &, QtPropertyData *)) );
	delete ui;
	delete propEditor;
	SafeRelease(entity);
	for(DAVA::Map<QWidget*, QWidget*>::iterator it = additionalWidgetMap.begin(); it != additionalWidgetMap.end(); ++it)
	{
		delete it->second;
	}
	additionalWidgetMap.clear();
}

void BaseAddEntityDialog::showEvent ( QShowEvent * event )
{
	QDialog::showEvent(event);
	propEditor->expandAll();
	PerformResize();
}

void BaseAddEntityDialog::PerformResize()
{
	propEditor->expandAll();
	int propEditorHeight = 0;
	if(propEditor->isVisible())
	{
		propEditorHeight = PROPERTY_EDITOR_HEIGHT;
	}

	QRect rectEditor = propEditor->geometry();
	rectEditor.setHeight(propEditorHeight);
	propEditor->setGeometry(rectEditor);

	QScrollArea* area = ui->scrollArea;
	int scrollAreaHeight = area->sizeHint().height();
	
	QRect areaEditor = area->geometry();
	areaEditor.setHeight(scrollAreaHeight);
	area->setGeometry(areaEditor);
	
	int currentMax  = ui->lowerLayOut->geometry().height() + ui->lowerLayOut->verticalSpacing() * 4 + scrollAreaHeight + propEditorHeight;
	int maxHeight = currentMax < WINDOW_HEIGHT_LIMIT ? currentMax : WINDOW_HEIGHT_LIMIT;

	int minHeight = propEditorHeight + ui->lowerLayOut->geometry().height() + ui->lowerLayOut->verticalSpacing() * 3;
	setMinimumHeight(minHeight);
	QRect rect = geometry();
	rect.setHeight(maxHeight);
	setGeometry(rect);
}

QtPropertyData* BaseAddEntityDialog::AddInspMemberToEditor(void *object, const DAVA::InspMember * member)
{
	int flags = DAVA::I_VIEW | DAVA::I_EDIT;
	QtPropertyData* propData = QtPropertyDataIntrospection::CreatePropDataFromInspMember(object, member, flags);
	propEditor->AppendProperty(member->Name(), propData);
	return propData;
}

QtPropertyData* BaseAddEntityDialog::AddKeyedArchiveMember(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::String& rowName)
{
	QtPropertyData*  propData = new QtPropertyKeyedArchiveMember(_archive, _key);
	propEditor->AppendProperty(rowName.c_str(), propData);
	return propData;
}

QtPropertyData* BaseAddEntityDialog::AddMetaObject(void *_object, const DAVA::MetaInfo *_meta, const String& rowName)
{
	QtPropertyData*  propData = new QtPropertyDataMetaObject( _object, _meta);
	propEditor->AppendProperty(rowName.c_str(), propData);
	return propData;
}

void BaseAddEntityDialog::SetEntity(DAVA::Entity* _entity)
{
	SafeRetain(_entity);
	SafeRelease(entity);
	
	entity = _entity;
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

void BaseAddEntityDialog::InitPropertyEditor()
{
	propEditor = new QtPropertyEditor(this);
	propEditor->setObjectName(QString::fromUtf8("propEditor"));
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHeightForWidth(propEditor->sizePolicy().hasHeightForWidth());
	propEditor->setSizePolicy(sizePolicy);
	propEditor->setMinimumSize(QSize(0, 0));
	propEditor->setMaximumSize(QSize(16777215, 9999));
	propEditor->setMouseTracking(false);
	propEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	propEditor->setTabKeyNavigation(false);
	propEditor->setAlternatingRowColors(true);
	propEditor->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	propEditor->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	propEditor->setIndentation(16);
	propEditor->setAnimated(false);
	propEditor->setVisible(true);
	propEditor->SetEditTracking(true);
	ui->verticalLayout_4->addWidget(propEditor);
	propEditor->setMinimumHeight(PROPERTY_EDITOR_HEIGHT);
	connect( propEditor, SIGNAL(PropertyEdited(const QString &, QtPropertyData *)), this, SLOT(OnItemEdited(const QString &, QtPropertyData *)) );
}

void BaseAddEntityDialog::OnItemEdited(const QString &name, QtPropertyData *data)
{
	Command2 *command = (Command2 *) data->CreateLastCommand();
	if(NULL != command)
	{
		SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
		if(NULL != curScene)
		{
			curScene->Exec(command);
		}
	}
}
