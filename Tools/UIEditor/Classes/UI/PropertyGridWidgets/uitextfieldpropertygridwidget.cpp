#include "uitextfieldpropertygridwidget.h"
#include "ui_uitextfieldpropertygridwidget.h"
#include "fontmanagerdialog.h"
#include "CommandsController.h"
#include "ChangePropertyCommand.h"
#include "ResourcesManageHelper.h"
#include "PropertyNames.h"
#include "PropertiesHelper.h"
#include "WidgetSignalsBlocker.h"
#include "BackgroundGridWidgetHelper.h"

using namespace PropertyNames;

static const QString TEXTFIELD_PROPERTY_BLOCK_NAME = "Text";

UITextFieldPropertyGridWidget::UITextFieldPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::UITextFieldPropertyGridWidget)
{
    ui->setupUi(this);
	SetPropertyBlockName(TEXTFIELD_PROPERTY_BLOCK_NAME);
}

UITextFieldPropertyGridWidget::~UITextFieldPropertyGridWidget()
{
    delete ui;
}

void UITextFieldPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
	FillComboboxes();
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    RegisterPushButtonWidgetForProperty(propertiesMap, FONT_PROPERTY_NAME, ui->fontSelectButton);
    RegisterSpinBoxWidgetForProperty(propertiesMap, FONT_SIZE_PROPERTY_NAME, ui->fontSizeSpinBox);
    
    RegisterLineEditWidgetForProperty(propertiesMap, TEXT_PROPERTY_NAME, ui->textLineEdit);
    RegisterColorButtonWidgetForProperty(propertiesMap, TEXT_COLOR_PROPERTY_NAME, ui->textColorPushButton);

    RegisterSpinBoxWidgetForProperty(propertiesMap, SHADOW_OFFSET_X, ui->shadowOffsetXSpinBox);
    RegisterSpinBoxWidgetForProperty(propertiesMap, SHADOW_OFFSET_Y, ui->shadowOffsetYSpinBox);
    RegisterColorButtonWidgetForProperty(propertiesMap, SHADOW_COLOR, ui->shadowColorButton);

	RegisterComboBoxWidgetForProperty(propertiesMap, TEXT_ALIGN_PROPERTY_NAME, ui->alignComboBox, false, true);
}

void UITextFieldPropertyGridWidget::Cleanup()
{
    UnregisterPushButtonWidget(ui->fontSelectButton);
    UnregisterSpinBoxWidget(ui->fontSizeSpinBox);
    UnregisterColorButtonWidget(ui->textColorPushButton);

    UnregisterSpinBoxWidget(ui->shadowOffsetXSpinBox);
    UnregisterSpinBoxWidget(ui->shadowOffsetYSpinBox);
    UnregisterColorButtonWidget(ui->shadowColorButton);

	UnregisterComboBoxWidget(ui->alignComboBox);
    
    BasePropertyGridWidget::Cleanup();
}

void UITextFieldPropertyGridWidget::ProcessPushButtonClicked(QPushButton *senderWidget)
{
	    if ((activeMetadata == NULL) || (senderWidget != this->ui->fontSelectButton))
    {
        // No control already assinged or not fontSelectButton
        return;
    }
    
	// Get current value of Font property
	Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata, FONT_PROPERTY_NAME, false);
	// Get sprite path from graphics font
	QString currentGFontPath = ResourcesManageHelper::GetGraphicsFontPath(fontPropertyValue);

    //Call font selection dialog - with ok button and preset of graphics font path
    FontManagerDialog *fontDialog = new FontManagerDialog(true, currentGFontPath);
    Font *resultFont = NULL;
    
    if ( fontDialog->exec() == QDialog::Accepted )
    {
        resultFont = fontDialog->ResultFont();
    }
    
    //Delete font select dialog reference
    SafeDelete(fontDialog);
    
    if (!resultFont)
    {
        return;
    }
    
    PROPERTYGRIDWIDGETSITER iter = propertyGridWidgetsMap.find(senderWidget);
    if (iter == propertyGridWidgetsMap.end())
    {
        Logger::Error("OnPushButtonClicked - unable to find attached property in the propertyGridWidgetsMap!");
        return;
    }
    
	// Don't update the property if the text wasn't actually changed.
    Font* curValue = PropertiesHelper::GetAllPropertyValues<Font*>(this->activeMetadata, iter->second.getProperty().name());
	if (curValue->IsEqual(resultFont))
	{
		SafeRelease(resultFont);
		return;
	}

    BaseCommand* command = new ChangePropertyCommand<Font *>(activeMetadata, iter->second, resultFont);
    CommandsController::Instance()->ExecuteCommand(command);
    SafeRelease(command);
	// TODO - probable memory leak. Need to investigate how to fix it
	// SafeRelease(resultFont);
}

void UITextFieldPropertyGridWidget::UpdatePushButtonWidgetWithPropertyValue(QPushButton *pushButtonWidget, const QMetaProperty &curProperty)
{
    
    if (pushButtonWidget != this->ui->fontSelectButton)
    {
        return; //Not font select button
    }
    
    bool isPropertyValueDiffers = false;
    Font *fontPropertyValue = PropertiesHelper::GetPropertyValue<Font *>(this->activeMetadata,
                                                                         curProperty.name(), isPropertyValueDiffers);
    if (fontPropertyValue)
    {
        //Set button text
        WidgetSignalsBlocker blocker(pushButtonWidget);
        Font::eFontType fontType = fontPropertyValue->GetFontType();
        QString buttonText;
        
        switch (fontType)
        {
            case Font::TYPE_FT:
            {
                FTFont *ftFont = dynamic_cast<FTFont*>(fontPropertyValue);
                //Set pushbutton widget text
                buttonText = QString::fromStdString(ftFont->GetFontPath().GetAbsolutePathname());
                break;
            }
            case Font::TYPE_GRAPHICAL:
            {
                GraphicsFont *gFont = dynamic_cast<GraphicsFont*>(fontPropertyValue);
                //Put into result string font definition and font sprite path
                Sprite *fontSprite = gFont->GetFontSprite();
                if (!fontSprite) //If no sprite available - quit
                {
                    pushButtonWidget->setText("Graphical font is not available");
                    return;
                }
                //Get font definition and sprite relative path
                QString fontDefinitionName = QString::fromStdString(gFont->GetFontDefinitionName().GetAbsolutePathname());
                QString fontSpriteName =QString::fromStdString(fontSprite->GetRelativePathname().GetAbsolutePathname());
                //Set push button widget text - for grapics font it contains font definition and sprite names
                buttonText = QString("%1\n%2").arg(fontDefinitionName, fontSpriteName);
                break;
            }
            default:
            {
                //Do nothing if we can't determine font type
                return;
            }
        }
        
        pushButtonWidget->setText(buttonText);
    }
}


void UITextFieldPropertyGridWidget::FillComboboxes()
{
    ui->alignComboBox->clear();
    int itemsCount = BackgroundGridWidgetHelper::GetAlignTypesCount();
    for (int i = 0; i < itemsCount; i ++)
    {
        ui->alignComboBox->addItem(BackgroundGridWidgetHelper::GetAlignTypeDesc(i));
    }

}

void UITextFieldPropertyGridWidget::CustomProcessComboboxValueChanged(const PROPERTYGRIDWIDGETSITER& iter, int value)
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

void UITextFieldPropertyGridWidget::ProcessComboboxValueChanged(QComboBox* senderWidget, const PROPERTYGRIDWIDGETSITER& iter,
                                             const QString& value)
{
	if (senderWidget == NULL)
    {
        Logger::Error("UITextFieldPropertyGridWidget::ProcessComboboxValueChanged: senderWidget is NULL!");
        return;
    }
    
    // Try to process this control-specific widgets.
    int selectedIndex = senderWidget->currentIndex();

	if (senderWidget == ui->alignComboBox)
    {
        return CustomProcessComboboxValueChanged(iter, BackgroundGridWidgetHelper::GetAlignType(selectedIndex));
    }

    // No postprocessing was applied - use the generic process.
    BasePropertyGridWidget::ProcessComboboxValueChanged(senderWidget, iter, value);
}

void UITextFieldPropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(QComboBox* comboBoxWidget, const QMetaProperty& curProperty)
{
	if (!this->activeMetadata)
    {
        return;
    }

    bool isPropertyValueDiffers = false;
    const QString& propertyName = curProperty.name();
    int propertyValue = PropertiesHelper::GetPropertyValue<int>(this->activeMetadata, propertyName, isPropertyValueDiffers);

    // Firstly check the custom comboboxes.
    if (comboBoxWidget == ui->alignComboBox)
    {
        UpdateWidgetPalette(comboBoxWidget, propertyName);
        return SetComboboxSelectedItem(comboBoxWidget,
                                       BackgroundGridWidgetHelper::GetAlignTypeDescByType(propertyValue));
    }

    // Not related to the custom combobox - call the generic one.
    BasePropertyGridWidget::UpdateComboBoxWidgetWithPropertyValue(comboBoxWidget, curProperty);
}
