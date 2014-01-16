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

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QAction>
#include <QVariant>

#include "MaterialEditor.h"
#include "ui_materialeditor.h"

#include "MaterialModel.h"
#include "MaterialFilterModel.h"

#include "Main/mainwindow.h"
#include "Main/QtUtils.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspDynamic.h"
#include "Tools/QtWaitDialog/QtWaitDialog.h"

MaterialEditor::MaterialEditor(QWidget *parent /* = 0 */)
: QDialog(parent)
, ui(new Ui::MaterialEditor)
, curMaterial(NULL)
, templatesScaned(false)
{
	ui->setupUi(this);
	setWindowFlags(WINDOWFLAG_ON_TOP_OF_APPLICATION);

	ui->materialTree->setDragEnabled(true);
	ui->materialTree->setAcceptDrops(true);
	ui->materialTree->setDragDropMode(QAbstractItemView::DragDrop);

	ui->materialProperty->SetEditTracking(true);

	// global scene manager signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
	
	// material tree
	QObject::connect(ui->materialTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(materialSelected(const QItemSelection &, const QItemSelection &)));

	// material properties
	QObject::connect(ui->materialProperty, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnPropertyEdited(const QModelIndex &)));
	QObject::connect(ui->templateBox, SIGNAL(activated(int)), this, SLOT(OnTemplateChanged(int)));

	posSaver.Attach(this);
	posSaver.LoadState(ui->splitter);
	posSaver.LoadState(ui->splitter_2);

	DAVA::VariantType v1 = posSaver.LoadValue("splitPosProperties");
	DAVA::VariantType v2 = posSaver.LoadValue("splitPosTexttures");
	if(v1.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialProperty->header()->resizeSection(0, v1.AsInt32());
	if(v2.GetType() == DAVA::VariantType::TYPE_INT32) ui->materialTexture->header()->resizeSection(0, v2.AsInt32());

    initActions();
}

MaterialEditor::~MaterialEditor()
{ 
	DAVA::VariantType v1(ui->materialProperty->header()->sectionSize(0));
	DAVA::VariantType v2(ui->materialTexture->header()->sectionSize(0));
	posSaver.SaveValue("splitPosProperties", v1);
	posSaver.SaveValue("splitPosTexttures", v2);

	posSaver.SaveState(ui->splitter);
	posSaver.SaveState(ui->splitter_2);
}

void MaterialEditor::initActions()
{
    MaterialFilteringModel *model = qobject_cast< MaterialFilteringModel * >( ui->materialTree->model() );
    Q_ASSERT( model );
    if ( !model )
        return ;

    ui->actionShowAll->setData( MaterialFilteringModel::SHOW_ALL );
    ui->actionInstances->setData( MaterialFilteringModel::SHOW_ONLY_INSTANCES );
    ui->actionMaterialsInstances->setData( MaterialFilteringModel::SHOW_INSTANCES_AND_MATERIALS );

    const int filterType =  model->getFilterType();
    foreach( QAction *action, ui->filterType->actions() )
    {
        connect( action, SIGNAL( triggered() ), SLOT( onFilterChanged() ) );
        if ( action->data().toInt() == filterType )
            action->activate( QAction::Trigger );
    }

    connect( ui->actionAutoExpand, SIGNAL( triggered( bool ) ), ui->materialTree, SLOT( onCurrentExpandModeChange( bool ) ) );
}

void MaterialEditor::onFilterChanged()
{
    MaterialFilteringModel *model = qobject_cast< MaterialFilteringModel * >( ui->materialTree->model() );
    Q_ASSERT( model );
    if ( !model )
        return ;

    QAction *action = ui->filterType->checkedAction();
    if ( action == NULL )
        return ;
    const int filterType = action->data().toInt();
    model->setFilterType( filterType );

    ui->actionAutoExpand->setChecked( ui->materialTree->currentExpandMode() );
}

void MaterialEditor::SelectMaterial(DAVA::NMaterial *material)
{
	ui->materialTree->SelectMaterial(material);
}

void MaterialEditor::SelectEntities(DAVA::NMaterial *material)
{
	ui->materialTree->SelectEntities(material);
}

void MaterialEditor::SetCurMaterial(DAVA::NMaterial *material)
{
	curMaterial = material;

	// Don't remove properties immediately. Just extract them and remove later
	// this should be done, because we want to remove all properties when propertyEdited signal emited
	{
		QtPropertyData *propRoot = ui->materialProperty->GetRootProperty();
		while(propRoot->ChildCount() > 0)
		{
			QtPropertyData *child = propRoot->ChildGet(0);
			propRoot->ChildExtract(child);

			child->deleteLater();
		}

		propRoot = ui->materialTexture->GetRootProperty();
		while(propRoot->ChildCount() > 0)
		{
			QtPropertyData *child = propRoot->ChildGet(0);
			propRoot->ChildExtract(child);

			child->deleteLater();
		}
	}

	//ui->materialProperty->RemovePropertyAll();
	//ui->materialTexture->RemovePropertyAll();

	if(NULL != material)
	{
		FillMaterialProperties(material);
		FillMaterialTextures(material);

		// set current template
		DAVA::FilePath curMaterialTemplate = material->GetMaterialTemplate()->name.c_str();
		int curIndex = templates.indexOf(curMaterialTemplate);

		if(-1 != curIndex)
		{
			ui->templateBox->setCurrentIndex(curIndex);
		}
		else
		{
			ui->templateBox->setCurrentIndex(0);
		}

		// enable template selection only for real materials, not instances
		ui->templateBox->setEnabled(DAVA::NMaterial::MATERIALTYPE_MATERIAL == material->GetMaterialType());
	}
}

void MaterialEditor::sceneActivated(SceneEditor2 *scene)
{
	if(isVisible())
	{
		ui->materialTree->SetScene(scene);
	}
}

void MaterialEditor::sceneDeactivated(SceneEditor2 *scene)
{ 
	ui->materialTree->SetScene(NULL);
	SetCurMaterial(NULL);
}

void MaterialEditor::materialSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
	if(1 == selected.indexes().size())
	{
		DAVA::NMaterial *material = ui->materialTree->GetMaterial(selected.indexes().at(0));
		SetCurMaterial(material);
	}
	else
	{
		SetCurMaterial(NULL);
	}
}

void MaterialEditor::showEvent(QShowEvent * event)
{
	sceneActivated(QtMainWindow::Instance()->GetCurrentScene());

	if(!templatesScaned)
	{
		ScanTemplates();
	}
}

void MaterialEditor::FillMaterialProperties(DAVA::NMaterial *material)
{
	const DAVA::InspInfo *info = material->GetTypeInfo();
	const DAVA::InspMember *materialProperties = info->Member("materialProperties");
	const DAVA::InspMember *materialFlags = info->Member("materialSetFlags");

	// fill material name
	const DAVA::InspMember *nameMember = info->Member("materialName");
	if(NULL != nameMember)
	{
		QtPropertyDataInspMember *name = new QtPropertyDataInspMember(material, nameMember);
		ui->materialProperty->AppendProperty("Name", name);
	}

	// fill material flags
	if(NULL != materialFlags)
	{
		const DAVA::InspMemberDynamic* dynamicInsp = materialFlags->Dynamic();

		if(NULL != dynamicInsp)
		{
			DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
			DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow

			for(size_t i = 0; i < membersList.size(); ++i)
			{
				QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);
				ui->materialProperty->AppendProperty(membersList[i].c_str(), dynamicMember);
			}
		}
	}

	// fill material properties
	if(NULL != materialProperties)
	{
		const DAVA::InspMemberDynamic* dynamicInsp = materialProperties->Dynamic();

		if(NULL != dynamicInsp)
		{
			DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
			DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow

			for(size_t i = 0; i < membersList.size(); ++i)
			{
				int memberFlags = dynamicInfo->MemberFlags(material, membersList[i]);
				QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);

				// self property
				if(memberFlags & DAVA::I_EDIT)
				{
					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cminus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemProperty()));

					// isn't set in parent or shader
					if(!(memberFlags & DAVA::I_VIEW) && !(memberFlags & DAVA::I_SAVE))
					{
						dynamicMember->SetBackground(QBrush(QColor(255, 0, 0, 10)));
					}
				}
				// not self property (is set in parent or shader)
				else
				{
					// disable property and it childs
					dynamicMember->SetEnabled(false);
					for(int m = 0; m < dynamicMember->ChildCount(); ++m)
					{
						dynamicMember->ChildGet(m)->SetEnabled(false);
					}

					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cplus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddProperty()));

					dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));

					// required by shader
					//if(!(memberFlags & DAVA::I_VIEW) && (memberFlags & DAVA::I_SAVE))
					//{	}
				}

				ui->materialProperty->AppendProperty(membersList[i].c_str(), dynamicMember);
			}
		}
	}
}

void MaterialEditor::FillMaterialTextures(DAVA::NMaterial *material)
{
	const DAVA::InspInfo *info = material->GetTypeInfo();
	const DAVA::InspMember *materialTextures = info->Member("textures");

	// fill own material textures
	if(NULL != materialTextures)
	{
		const DAVA::InspMemberDynamic* dynamicInsp = materialTextures->Dynamic();

		if(NULL != dynamicInsp)
		{
			DAVA::InspInfoDynamic *dynamicInfo = dynamicInsp->GetDynamicInfo();
			DAVA::Vector<DAVA::FastName> membersList = dynamicInfo->MembersList(material); // this function can be slow

			for(size_t i = 0; i < membersList.size(); ++i)
			{
				int memberFlags = dynamicInfo->MemberFlags(material, membersList[i]);
				QtPropertyDataInspDynamic *dynamicMember = new QtPropertyDataInspDynamic(material, dynamicInfo, membersList[i]);

				// self property
				if(memberFlags & DAVA::I_EDIT)
				{
					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cminus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnRemTexture()));

					// isn't set in parent or shader
					if(!(memberFlags & DAVA::I_VIEW) && !(memberFlags & DAVA::I_SAVE))
					{
						dynamicMember->SetBackground(QBrush(QColor(255, 0, 0, 10)));
					}
				}
				// not self property (is set in parent or shader)
				else
				{
					// disable property and it childs
					dynamicMember->SetEnabled(false);
					for(int m = 0; m < dynamicMember->ChildCount(); ++m)
					{
						dynamicMember->ChildGet(m)->SetEnabled(false);
					}

					QtPropertyToolButton* btn = dynamicMember->AddButton();
					btn->setIcon(QIcon(":/QtIcons/cplus.png"));
					btn->setIconSize(QSize(14, 14));
					QObject::connect(btn, SIGNAL(clicked()), this, SLOT(OnAddTexture()));

					dynamicMember->SetBackground(QBrush(QColor(0, 0, 0, 10)));
				}

				ui->materialTexture->AppendProperty(membersList[i].c_str(), dynamicMember);
			}
		}
	}
}

void MaterialEditor::ScanTemplates()
{
	int i = 0;
	QtWaitDialog waitDlg;
	waitDlg.Show("Scanning material templates", "", true, false);

	templates.clear();
	ui->templateBox->clear();

	// add unknown template
	templates.append("");
	ui->templateBox->addItem("Unknown", i++);

	DAVA::FilePath materialsListPath = DAVA::FilePath("~res:/Materials/Legacy/assignable.txt");
	if(materialsListPath.Exists())
	{
		QString materialsListDir = materialsListPath.GetDirectory().GetAbsolutePathname().c_str();

		// scan for known templates
		QFile materialsListFile(materialsListPath.GetAbsolutePathname().c_str());
		if(materialsListFile.open(QIODevice::ReadOnly))
		{
			QTextStream in(&materialsListFile);
			while(!in.atEnd())
			{
				QFileInfo materialPath(materialsListDir + in.readLine());
				if(materialPath.exists())
				{
					templates.append(DAVA::FilePath(materialPath.absoluteFilePath().toAscii().data()).GetFrameworkPath());
					ui->templateBox->addItem(materialPath.completeBaseName(), i++);
				}
			}
			materialsListFile.close();
		}
	}

	waitDlg.Reset();
	templatesScaned = true;
}

void MaterialEditor::OnAddProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());
	
	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnRemProperty()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnAddTexture()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(data->GetValue(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnRemTexture()
{
	QtPropertyToolButton *btn = dynamic_cast<QtPropertyToolButton *>(QObject::sender());

	if(NULL != btn && NULL != curMaterial)
	{
		QtPropertyDataInspDynamic *data = dynamic_cast<QtPropertyDataInspDynamic *>(btn->GetPropertyData());
		if(NULL != data)
		{
			data->SetValue(QVariant(), QtPropertyData::VALUE_EDITED);

			// reload material properties
			SetCurMaterial(curMaterial);
		}
	}
}

void MaterialEditor::OnTemplateChanged(int index)
{
	if(NULL != curMaterial && index > 0 && index < templates.size())
	{
		DAVA::FilePath newTemplateName = templates[index];
		DAVA::NMaterialHelper::SwitchTemplate(curMaterial, DAVA::FastName(newTemplateName.GetFrameworkPath().c_str()));
	}

	SetCurMaterial(curMaterial);
}

void MaterialEditor::OnPropertyEdited(const QModelIndex &index)
{
	// reload material properties
	SetCurMaterial(curMaterial);
	ui->materialTree->Update();
}
