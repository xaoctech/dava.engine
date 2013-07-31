#include "ActionComponentEditor.h"
#include "ui_ActionComponentEditor.h"
#include "../Qt/Main/QtUtils.h"

#include <QTableWidgetItem>
#include <QComboBox>
#include <QSpinBox>

#define COLUMN_ACTION_TYPE 0
#define COLUMN_ENTITY_NAME 1
#define COLUMN_DELAY 2
#define COLUMN_SWITCH_INDEX 3

const int ACTION_NAME_COUNT = 3;
static QString ACTION_TYPE_NAME[] =
{
	"None",
	"Particle",
	"Sound"
};

ActionComponentEditor::ActionComponentEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ActionComponentEditor)
{
    ui->setupUi(this);
	
	targetComponent = NULL;
	QObject::connect(ui->buttonAddItem, SIGNAL(pressed()), this, SLOT(OnAddAction()));
	QObject::connect(ui->buttonRemoveItem, SIGNAL(pressed()), this, SLOT(OnRemoveAction()));
	QObject::connect(ui->tableActions, SIGNAL(itemSelectionChanged()), this, SLOT(OnSelectedItemChanged()));
	
	ui->tableActions->resizeColumnsToContents();
	ui->buttonRemoveItem->setEnabled(false);
	
	editDelegate.setParent(ui->tableActions);
	editDelegate.SetComponentEditor(this);
	ui->tableActions->setItemDelegate(&editDelegate);
}

ActionComponentEditor::~ActionComponentEditor()
{
    delete ui;
}

void ActionComponentEditor::SetComponent(DAVA::ActionComponent* component)
{
	editDelegate.SetComponent(component);
	targetComponent = component;
	UpdateTableFromComponent(targetComponent);
	
	ui->buttonAddItem->setEnabled(!IsActionPresent(GetDefaultAction()));
}

void ActionComponentEditor::UpdateTableFromComponent(DAVA::ActionComponent* component)
{
	ui->tableActions->clearContents();
	
	int actionCount = component->GetCount();
	ui->tableActions->setRowCount(actionCount);
	
	if(actionCount > 0)
	{
		for(int i = 0; i < actionCount; ++i)
		{
			DAVA::ActionComponent::Action& action = component->Get(i);
			
			QTableWidgetItem* actionTypeTableItem = new QTableWidgetItem(ACTION_TYPE_NAME[action.type]);
			QTableWidgetItem* entityNameTableItem = new QTableWidgetItem(action.entityName.c_str());
			QTableWidgetItem* delayTableItem = new QTableWidgetItem(QString("%1").arg(action.delay, 16, 'f', 2));
			QTableWidgetItem* switchIndexTableItem = new QTableWidgetItem(QString("%1").arg(action.switchIndex));
			
			ui->tableActions->setItem(i, COLUMN_ACTION_TYPE, actionTypeTableItem);
			ui->tableActions->setItem(i, COLUMN_ENTITY_NAME, entityNameTableItem);
			ui->tableActions->setItem(i, COLUMN_DELAY, delayTableItem);
			ui->tableActions->setItem(i, COLUMN_SWITCH_INDEX, switchIndexTableItem);
		}
		
		ui->tableActions->resizeColumnsToContents();
		ui->tableActions->resizeRowsToContents();
	}
}

void ActionComponentEditor::OnAddAction()
{
	//add action with default values
	DAVA::ActionComponent::Action action = GetDefaultAction();
	
	bool duplicateAction = IsActionPresent(action);	
	if(duplicateAction)
	{
		ShowErrorDialog("Duplicate actions not allowed!");
	}
	else
	{
		targetComponent->Add(action);
		UpdateTableFromComponent(targetComponent);
		
		ui->buttonAddItem->setEnabled(false);
	}
}

void ActionComponentEditor::OnRemoveAction()
{
	int currentRow = ui->tableActions->currentRow();
	if(currentRow >= 0 &&
	   currentRow < targetComponent->GetCount())
	{
		targetComponent->Remove(targetComponent->Get(currentRow));
		UpdateTableFromComponent(targetComponent);
		
		bool itemsPresent = (targetComponent->GetCount() > 0);
		ui->buttonRemoveItem->setEnabled(itemsPresent);
		if(itemsPresent)
		{
			ui->tableActions->setCurrentCell(0, 0);
		}
		
		ui->buttonAddItem->setEnabled(!IsActionPresent(GetDefaultAction()));
	}
}

void ActionComponentEditor::OnSelectedItemChanged()
{
	int currentRow = ui->tableActions->currentRow();
	ui->buttonRemoveItem->setEnabled(currentRow >= 0);
}

DAVA::ActionComponent::Action ActionComponentEditor::GetDefaultAction()
{
	DAVA::ActionComponent::Action action;
	action.type = DAVA::ActionComponent::Action::TYPE_PARTICLE_EFFECT;
	action.entityName = targetComponent->GetEntity()->GetName();
	action.delay = 0.0f;
	action.switchIndex = -1;

	return action;
}

bool ActionComponentEditor::IsActionPresent(const DAVA::ActionComponent::Action action)
{
	bool actionPresent = false;
	for(int i = 0; i < targetComponent->GetCount(); ++i)
	{
		const DAVA::ActionComponent::Action& innerAction = targetComponent->Get(i);
		if(innerAction.switchIndex == action.switchIndex &&
		   innerAction.entityName == action.entityName &&
		   innerAction.type == action.type)
		{
			actionPresent = true;
			break;
		}
	}

	return actionPresent;
}

void ActionComponentEditor::Update()
{
	ui->tableActions->resizeColumnsToContents();
	ui->tableActions->resizeRowsToContents();

	ui->buttonAddItem->setEnabled(!IsActionPresent(GetDefaultAction()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ActionItemEditDelegate::ActionItemEditDelegate(QObject *parent) : targetComponent(NULL), componentEditor(NULL)
{	
}

QWidget* ActionItemEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
					  const QModelIndex &index) const
{	
	QWidget* editor = NULL;
	switch(index.column())
	{
		case COLUMN_ACTION_TYPE:
		{
			QComboBox* combo = new QComboBox(parent);
			combo->setFrame(false);
			for(int i = 1; i < ACTION_NAME_COUNT - 1; ++i) //do not add sound aciton
			{
				combo->addItem(ACTION_TYPE_NAME[i]);
			}
			
			editor = combo;
			
			break;
		}
			
		case COLUMN_ENTITY_NAME:
		{
			DAVA::Entity* parentEntity = targetComponent->GetEntity();
			DAVA::Vector<DAVA::Entity*> allChildren;
			parentEntity->GetChildNodes(allChildren);

			DAVA::Vector<DAVA::String> childrenNames;
			childrenNames.push_back(parentEntity->GetName());
			for(int i = 0; i < allChildren.size(); ++i)
			{
				childrenNames.push_back(allChildren[i]->GetName());
			}
			
			std::sort(childrenNames.begin(), childrenNames.end());
			childrenNames.erase(std::unique(childrenNames.begin(), childrenNames.end()), childrenNames.end());
			
			QComboBox* combo = new QComboBox(parent);
			combo->setFrame(false);
			for(int i = 0; i < childrenNames.size(); ++i)
			{
				combo->addItem(childrenNames[i].c_str());
			}
			
			editor = combo;
			
			break;
		}
			
		case COLUMN_DELAY:
		{
			QDoubleSpinBox* spinBox = new QDoubleSpinBox(parent);
			spinBox->setMinimum(0.0f);
			spinBox->setMaximum(3600.f);
			spinBox->setSingleStep(0.01f);
			
			editor = spinBox;
			
			break;
		}
			
		case COLUMN_SWITCH_INDEX:
		{
			QSpinBox* spinBox = new QSpinBox(parent);
			spinBox->setMinimum(-1);
			spinBox->setMaximum(128);
			spinBox->setSingleStep(1);
			
			editor = spinBox;

			break;
		}
	}

	DVASSERT(editor);
	return editor;
}

void ActionItemEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	const DAVA::ActionComponent::Action& currentAction = targetComponent->Get(index.row());

	switch(index.column())
	{
		case COLUMN_ACTION_TYPE:
		{
			QComboBox* combo = static_cast<QComboBox*>(editor);
			combo->setCurrentIndex((int)currentAction.type - 1);
			
			break;
		}
			
		case COLUMN_ENTITY_NAME:
		{
			QComboBox* combo = static_cast<QComboBox*>(editor);
			for(int i = 0; i < combo->count(); ++i)
			{
				if(combo->itemText(i) == currentAction.entityName.c_str())
				{
					combo->setCurrentIndex(i);
					break;
				}
			}
			
			break;
		}
			
		case COLUMN_DELAY:
		{
			QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
			spinBox->setValue(currentAction.delay);
			
			break;
		}
			
		case COLUMN_SWITCH_INDEX:
		{
			QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
			spinBox->setValue(currentAction.switchIndex);
			
			break;
		}
	}

}

void ActionItemEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
				  const QModelIndex &index) const
{
	DAVA::ActionComponent::Action& currentAction = targetComponent->Get(index.row());
	
	switch(index.column())
	{
		case COLUMN_ACTION_TYPE:
		{
			QComboBox* combo = static_cast<QComboBox*>(editor);
			currentAction.type = (DAVA::ActionComponent::Action::eType)(combo->currentIndex() + 1);
			model->setData(index, combo->currentText(), Qt::EditRole);
			
			break;
		}
			
		case COLUMN_ENTITY_NAME:
		{
			QComboBox* combo = static_cast<QComboBox*>(editor);
			currentAction.entityName = combo->currentText().toStdString();
			model->setData(index, combo->currentText(), Qt::EditRole);
			
			break;
		}
			
		case COLUMN_DELAY:
		{
			QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
			currentAction.delay = (DAVA::float32)spinBox->value();
			model->setData(index, QString("%1").arg(currentAction.delay, 16, 'f', 2), Qt::EditRole);
			
			break;
		}
			
		case COLUMN_SWITCH_INDEX:
		{
			QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
			currentAction.switchIndex = spinBox->value();
			model->setData(index, QString("%1").arg(currentAction.switchIndex), Qt::EditRole);
			
			break;
		}
	}
	
	componentEditor->Update();
}

void ActionItemEditDelegate::updateEditorGeometry(QWidget *editor,
						  const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
}

void ActionItemEditDelegate::SetComponent(DAVA::ActionComponent* component)
{
	targetComponent = component;
}

void ActionItemEditDelegate::SetComponentEditor(ActionComponentEditor* editor)
{
	componentEditor = editor;
}

