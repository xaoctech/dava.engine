#include "backgroundpropertygridwidget.h"
#include "ui_backgroundpropertygridwidget.h"

#include <QColorDialog>
#include <QFileDialog>

#include "PropertyNames.h"
#include "BackgroundGridWidgetHelper.h"
#include "WidgetSignalsBlocker.h"

#include "ChangePropertyCommand.h"
#include "CommandsController.h"

#include "UIStaticTextMetadata.h"
#include "ResourcesManageHelper.h"

#include "ResourcePacker.h"

static const QString TEXT_PROPERTY_BLOCK_NAME = "Background";

BackGroundPropertyGridWidget::BackGroundPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::BackGroundPropertyGridWidget)
{
    ui->setupUi(this);
    ConnectToSignals();

    SetPropertyBlockName(TEXT_PROPERTY_BLOCK_NAME);
	
	BasePropertyGridWidget::InstallEventFiltersForWidgets(this);
}

BackGroundPropertyGridWidget::~BackGroundPropertyGridWidget()
{
    delete ui;
}

void BackGroundPropertyGridWidget::ConnectToSignals()
{
    //Connect button Clicked event to select color dialog action and open sprite dialog action
    connect(ui->openSpriteButton, SIGNAL(clicked()), this, SLOT(OpenSpriteDialog()));
    connect(ui->removeSpriteButton, SIGNAL(clicked()), this, SLOT(RemoveSprite()));
}

void BackGroundPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    FillComboboxes();
    
    RegisterGridWidgetAsStateAware();
    
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterLineEditWidgetForProperty(propertiesMap, PropertyNames::SPRITE_PROPERTY_NAME, ui->spriteLineEdit, false, true);
    RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::SPRITE_FRAME_PROPERTY_NAME, this->ui->frameSpinBox, false, true);
	RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::SPRITE_MODIFICATION_PROPERTY_NAME, this->ui->modificationComboBox, false, true);

	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::STRETCH_HORIZONTAL_PROPERTY_NAME, this->ui->lrSpinBox, false, true);
	RegisterSpinBoxWidgetForProperty(propertiesMap, PropertyNames::STRETCH_VERTICAL_PROPERTY_NAME, this->ui->tbSpinBox, false, true);
    
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::DRAW_TYPE_PROPERTY_NAME, ui->drawTypeComboBox, false, true);
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::COLOR_INHERIT_TYPE_PROPERTY_NAME, ui->colorInheritComboBox, false, true);
    RegisterComboBoxWidgetForProperty(propertiesMap, PropertyNames::ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);

    RegisterColorButtonWidgetForProperty(propertiesMap, PropertyNames::BACKGROUND_COLOR_PROPERTY_NAME, ui->selectColorButton, false, true);

    // Editing of sprites is not allowed for UIStaticText.
    bool disableSpriteEditingControls = (dynamic_cast<UIStaticTextMetadata*>(activeMetadata) != NULL);
    ui->spriteLineEdit->setDisabled(disableSpriteEditingControls);
    ui->frameSpinBox->setDisabled(disableSpriteEditingControls);
    ui->openSpriteButton->setDisabled(disableSpriteEditingControls);
	HandleDrawTypeComboBox();
}

void BackGroundPropertyGridWidget::Cleanup()
{
    UnregisterGridWidgetAsStateAware();

    UnregisterLineEditWidget(ui->spriteLineEdit);
    UnregisterSpinBoxWidget(ui->frameSpinBox);
    UnregisterComboBoxWidget(ui->drawTypeComboBox);
    UnregisterComboBoxWidget(ui->colorInheritComboBox);
    UnregisterComboBoxWidget(ui->alignComboBox);
    UnregisterColorButtonWidget(ui->selectColorButton);

	UnregisterComboBoxWidget(ui->modificationComboBox);
	UnregisterSpinBoxWidget(ui->lrSpinBox);
	UnregisterSpinBoxWidget(ui->tbSpinBox);

    BasePropertyGridWidget::Cleanup();
}

void BackGroundPropertyGridWidget::FillComboboxes()
{
    WidgetSignalsBlocker drawTypeBlocker(ui->drawTypeComboBox);
    WidgetSignalsBlocker colorInheritTypeBlocker(ui->colorInheritComboBox);
    WidgetSignalsBlocker alignBlocker(ui->alignComboBox);
	WidgetSignalsBlocker spriteModificationBlocker(ui->modificationComboBox);

    ui->drawTypeComboBox->clear();
    int itemsCount = BackgroundGridWidgetHelper::GetDrawTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->drawTypeComboBox->addItem(BackgroundGridWidgetHelper::GetDrawTypeDesc(i));
    }
    
    ui->colorInheritComboBox->clear();
    itemsCount = BackgroundGridWidgetHelper::GetColorInheritTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->colorInheritComboBox->addItem(BackgroundGridWidgetHelper::GetColorInheritTypeDesc(i));
    }
    
    ui->alignComboBox->clear();
    itemsCount = BackgroundGridWidgetHelper::GetAlignTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->alignComboBox->addItem(BackgroundGridWidgetHelper::GetAlignTypeDesc(i));
    }

	ui->modificationComboBox->clear();
    itemsCount = BackgroundGridWidgetHelper::GetModificationTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->modificationComboBox->addItem(BackgroundGridWidgetHelper::GetModificationTypeDesc(i));
    }
}

void BackGroundPropertyGridWidget::OpenSpriteDialog()
{
	// Pack all available sprites each time user open sprite dialog
	ResourcePacker *resPacker = new ResourcePacker();
	resPacker->PackResources(ResourcesManageHelper::GetSpritesDatasourceDirectory().toStdString(),
	 					 				ResourcesManageHelper::GetSpritesDirectory().toStdString());
	// Get sprites directory to open
	QString currentSpriteDir = ResourcesManageHelper::GetDefaultSpritesPath(this->ui->spriteLineEdit->text());
	// Get sprite path from file dialog
    QString spriteName = QFileDialog::getOpenFileName( this, tr( "Choose a sprite file" ),
															currentSpriteDir,
															tr( "Sprites (*.txt)" ) );
	if(!spriteName.isNull() && !spriteName.isEmpty())
    {
		// Convert file path into Unix-style path
		spriteName = ResourcesManageHelper::ConvertPathToUnixStyle(spriteName);

		if (ResourcesManageHelper::ValidateResourcePath(spriteName))
        {
            WidgetSignalsBlocker blocker(ui->spriteLineEdit);
			
            // Sprite name should be pre-processed to use relative path.
            ui->spriteLineEdit->setText(PreprocessSpriteName(spriteName));
            HandleLineEditEditingFinished(ui->spriteLineEdit);
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(spriteName);
		}
    }
	
	SafeDelete(resPacker);
}

void BackGroundPropertyGridWidget::RemoveSprite()
{
    //When we pass empty spriteLineEdit to command - this will cause removal of sprite
    if (!ui->spriteLineEdit->text().isEmpty())
    {
        WidgetSignalsBlocker blocker(ui->spriteLineEdit);
        ui->spriteLineEdit->setText("");
        HandleLineEditEditingFinished(ui->spriteLineEdit);
    }
}

void BackGroundPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                         const QString& value)
{
    if (senderWidget == NULL)
    {
        Logger::Error("BackGroundPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();
    
    if (senderWidget == ui->drawTypeComboBox)
    {
		HandleDrawTypeComboBox();
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetDrawType(selectedIndex));
    }
    else if (senderWidget == ui->colorInheritComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetColorInheritType(selectedIndex));
    }
    else if (senderWidget == ui->alignComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAlignType(selectedIndex));
    }
	else if (senderWidget == ui->modificationComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetModificationType(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void BackGroundPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
{
	// Don't update the property if the text wasn't actually changed.
    int curValue = PropertiesHelper::GetAllPropertyValues<int>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue == value)
	{
		return;
	}

	BaseCommand* command = new ChangePropertyCommand<int>(activeMetadata, iter->second, value);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
}

void BackGroundPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
    if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->drawTypeComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetDrawTypeDescByType((UIControlBackground::eDrawType)propertyValue));
    }
    else if (comboBoxWidget == ui->colorInheritComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetColorInheritTypeDescByType((UIControlBackground::eColorInheritType)
                                       propertyValue));
    }
    else if (comboBoxWidget == ui->alignComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetAlignTypeDescByType(propertyValue));
    }
	else if (comboBoxWidget == ui->modificationComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetModificationTypeDescByType(propertyValue));
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}

QString BackGroundPropertyGridWidget::PreprocessSpriteName(const QString& rawSpriteName)
{    
    return ResourcesManageHelper::GetResourceRelativePath(rawSpriteName);
}

void BackGroundPropertyGridWidget::HandleDrawTypeComboBox()
{
	if(NULL == ui->drawTypeComboBox)
	{
		return;
	}

	int selectedIndex = ui->drawTypeComboBox->currentIndex();
	UIControlBackground::eDrawType drawType = BackgroundGridWidgetHelper::GetDrawType(selectedIndex);
	
	bool lrState = false;
	bool tbState = false;
	bool modificationComboBoxState = true;
	bool alignComboBoxState = false;
	
	switch (drawType)
	{
		case UIControlBackground::DRAW_STRETCH_HORIZONTAL:
			lrState = true;
			tbState = false;
			modificationComboBoxState = false;
			break;
		case UIControlBackground::DRAW_STRETCH_VERTICAL:
			lrState = false;
			tbState = true;
			modificationComboBoxState = false;
			break;
		case UIControlBackground::DRAW_STRETCH_BOTH:
			lrState = true;
			tbState = true;
			modificationComboBoxState = false;
			break;
		case UIControlBackground::DRAW_ALIGNED:
			alignComboBoxState = true;
			break;
		default:
			break;
	}

	ui->lrSpinBox->setEnabled(lrState);
	ui->tbSpinBox->setEnabled(tbState);
	ui->modificationComboBox->setEnabled(modificationComboBoxState);
	ui->alignComboBox->setEnabled(alignComboBoxState);
}
