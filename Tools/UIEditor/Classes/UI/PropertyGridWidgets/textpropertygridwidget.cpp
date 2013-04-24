#include "textpropertygridwidget.h"
#include "ui_uitextfieldpropertygridwidget.h"

#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "UITextControlMetadata.h"
#include "PropertiesHelper.h"
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "WidgetSignalsBlocker.h"
#include "fontmanagerdialog.h"
#include "PropertyNames.h"
#include "ResourcesManageHelper.h"
#include "BackgroundGridWidgetHelper.h"
#include "UIStaticTextMetadata.h"
#include "UITextFieldMetadata.h"
#include "UIButtonMetadata.h"

#include "StringUtils.h"

using namespace DAVA;
using namespace PropertyNames;

static const QString TEXT_PROPERTY_BLOCK_NAME = "Text";

TextPropertyGridWidget::TextPropertyGridWidget(QWidget *parent) :
    UITextFieldPropertyGridWidget(parent)
    //ui(new Ui::TextPropertyGridWidget)
{
    SetPropertyBlockName(TEXT_PROPERTY_BLOCK_NAME);
	InsertLocalizationFields();
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

TextPropertyGridWidget::~TextPropertyGridWidget()
{
	delete localizationKeyNameLineEdit;
	delete localizationKeyTextLineEdit;
}

void TextPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
	BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // All these properties are state-aware.
    RegisterSpinBoxWidgetForProperty(propertiesMap, FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox, false, true);
    RegisterPushButtonWidgetForProperty(propertiesMap, FONT_PROPERTY_NAME, ui->fontSelectButton, false, true);
    RegisterColorButtonWidgetForProperty(propertiesMap, FONT_COLOR_PROPERTY_NAME, ui->textColorPushButton, false, true);
    // Shadow properties are also state-aware
    RegisterSpinBoxWidgetForProperty(propertiesMap, SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox, false, true);
    RegisterColorButtonWidgetForProperty(propertiesMap, SHADOW_COLOR, ui->shadowColorButton, false, true);
    // Localized Text Key is handled through generic Property mechanism, but we need to update the
    // Localization Value widget each time Localization Key is changes.
    RegisterLineEditWidgetForProperty(propertiesMap, LOCALIZED_TEXT_KEY_PROPERTY_NAME, localizationKeyNameLineEdit, false, true);
	RegisterComboBoxWidgetForProperty(propertiesMap, TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);

	bool enableTextAlignComboBox = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UITextFieldMetadata*>(activeMetadata)	!= NULL||
									dynamic_cast<UIButtonMetadata*>(activeMetadata)		!= NULL);
	ui->alignComboBox->setEnabled(enableTextAlignComboBox);

    UpdateLocalizationValue();

    RegisterGridWidgetAsStateAware();
}

void TextPropertyGridWidget::InsertLocalizationFields()
{
	ui->textLineEdit->setEnabled(false);
	ui->textLineEdit->setVisible(false);
	ui->label->setVisible(false);
	
	this->resize(300, 262);
    this->setMinimumSize(QSize(300, 262));

	ui->groupBox->resize(300, 262);
    ui->groupBox->setMinimumSize(QSize(300, 262));

	localizationKeyNameLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyNameLineEdit->setObjectName(QString::fromUtf8("localizationKeyNameLineEdit"));
	localizationKeyNameLineEdit->setGeometry(QRect(10, 25, 281, 22));
	localizationKeyTextLineEdit = new QLineEdit(ui->groupBox);
	localizationKeyTextLineEdit->setObjectName(QString::fromUtf8("localizationKeyTextLineEdit"));
	localizationKeyTextLineEdit->setEnabled(false);
	localizationKeyTextLineEdit->setGeometry(QRect(10, 55, 281, 22));
	localizationKeyTextLineEdit->setReadOnly(true);

	ui->fontNameLabel->setGeometry(QRect(10, 95, 31, 16));
	ui->fontSizeSpinBox->setGeometry(QRect(234, 91, 57, 25));
	ui->fontSelectButton->setGeometry(QRect(50, 86, 181, 38));
	ui->fontColorLabel->setGeometry(QRect(10, 140, 71, 16));
	ui->textColorPushButton->setGeometry(QRect(85, 138, 205, 21));
	ui->shadowOffsetLabel->setGeometry(QRect(10, 172, 91, 16));
	ui->offsetYLabel->setGeometry(QRect(210, 172, 16, 16));
	ui->shadowColorLabel->setGeometry(QRect(10, 202, 91, 16));
	ui->shadowOffsetXSpinBox->setGeometry(QRect(140, 168, 57, 25));
	ui->shadowOffsetYSpinBox->setGeometry(QRect(230, 168, 57, 25));
	ui->shadowColorButton->setGeometry(QRect(105, 200, 185, 21));
	ui->offsetXLabel->setGeometry(QRect(120, 172, 16, 16));
	ui->AlignLabel->setGeometry(QRect(10, 236, 62, 16));
	ui->alignComboBox->setGeometry(QRect(80, 230, 209, 26));
}

void TextPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();
    UnregisterLineEditWidget(localizationKeyNameLineEdit);
    UITextFieldPropertyGridWidget::Cleanup();
}

void TextPropertyGridWidget::UpdateLocalizationValue()
{
    if ( !this->activeMetadata )
    {
        return;
    }
    
    // Key is known now - determine and set the value.
    QString localizationKey = this->localizationKeyNameLineEdit->text();
    WideString localizationValue = LocalizationSystem::Instance()->GetLocalizedString(QStrint2WideString(localizationKey));
	this->localizationKeyTextLineEdit->setText(WideString2QStrint(localizationValue));
    
    // Also update the "dirty" style for the "Value"
    PROPERTYGRIDWIDGETSITER iter = this->propertyGridWidgetsMap.find(this->localizationKeyNameLineEdit);
    if (iter != this->propertyGridWidgetsMap.end())
    {
        UpdateWidgetPalette(this->localizationKeyTextLineEdit, iter->second.getProperty().name());
    }
}

void TextPropertyGridWidget::HandleSelectedUIControlStatesChanged(const Vector<UIControl::eControlState>& newStates)
{
    // When the UI Control State is changed. we need to update Localization Key/Value.
    BasePropertyGridWidget::HandleSelectedUIControlStatesChanged(newStates);
    UpdateLocalizationValue();
}

void TextPropertyGridWidget::HandleChangePropertySucceeded(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertySucceeded(propertyName);
    
    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}

void TextPropertyGridWidget::HandleChangePropertyFailed(const QString& propertyName)
{
    BasePropertyGridWidget::HandleChangePropertyFailed(propertyName);

    if (IsWidgetBoundToProperty(this->localizationKeyNameLineEdit, propertyName))
    {
        // Localization Key is updated - update the value.
        UpdateLocalizationValue();
    }
}
