#include "statepropertygridwidget.h"
#include "ui_statepropertygridwidget.h"

#include "UIControlStateHelper.h"
#include "PropertiesGridController.h"
#include <QList>

static const QString STATE_PROPERTY_BLOCK_NAME = "State";

StatePropertyGridWidget::StatePropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::StatePropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(STATE_PROPERTY_BLOCK_NAME);
    ui->stateSelectComboBox->setItemDelegate(&stateComboboxItemDelegate);
	
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

StatePropertyGridWidget::~StatePropertyGridWidget()
{
    delete ui;
}

void StatePropertyGridWidget::FillStatesList()
{
    ui->stateSelectComboBox->clear();
    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState controlState = UIControlStateHelper::GetUIControlState(i);
        ui->stateSelectComboBox->addItem(UIControlStateHelper::GetUIControlStateName(controlState),
                                         QVariant(controlState));
    }

    MarkupDirtyStates();
}

// Initialize with the metatada assigned.
void StatePropertyGridWidget::Initialize(BaseMetadata* metaData)
{
    BasePropertyGridWidget::Initialize(metaData);
    FillStatesList();

    connect(this->ui->stateSelectComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnCurrrentIndexChanged(int)));
    connect(this, SIGNAL(SelectedStateChanged(UIControl::eControlState)),
            PropertiesGridController::Instance(),
            SLOT(OnSelectedStateChanged(UIControl::eControlState)));
    
    // Select the first state, emit the signal to update controller.
    int selectedStateIndex = UIControlStateHelper::GetDefaultControlStateIndex();
    ui->stateSelectComboBox->setCurrentIndex(selectedStateIndex);
    emit SelectedStateChanged(UIControlStateHelper::GetDefaultControlState());
}

void StatePropertyGridWidget::Cleanup()
{
    disconnect(this->ui->stateSelectComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(OnCurrrentIndexChanged(int)));
    disconnect(this, SIGNAL(SelectedStateChanged(UIControl::eControlState)),
            PropertiesGridController::Instance(),
            SLOT(OnSelectedStateChanged(UIControl::eControlState)));

    BasePropertyGridWidget::Cleanup();
}

void StatePropertyGridWidget::OnCurrrentIndexChanged(int index)
{
    const QString& selectedItemName = this->ui->stateSelectComboBox->itemText(index);
    UIControl::eControlState selectedState = UIControlStateHelper::GetUIControlStateValue(selectedItemName);
    
    emit SelectedStateChanged(selectedState);
}


void StatePropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    MarkupDirtyStates();
}

void StatePropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);
    MarkupDirtyStates();
}

void StatePropertyGridWidget::MarkupDirtyStates()
{
    QList<int> markedTextIndexesList;

    int statesCount = UIControlStateHelper::GetUIControlStatesCount();
    for (int i = 0; i < statesCount; i ++)
    {
        UIControl::eControlState controlState = UIControlStateHelper::GetUIControlState(i);
        if (this->activeMetadata->IsStateDirty(controlState))
        {
            markedTextIndexesList.append(i);
        }
    }

    stateComboboxItemDelegate.SetBoldTextIndexesList(markedTextIndexesList);
}