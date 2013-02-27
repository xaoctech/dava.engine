#include "localizationeditordialog.h"
#include "ui_localizationeditordialog.h"
#include "LocalizationSystemHelper.h"
#include <QFileDialog>
#include "PropertiesGridController.h"
#include "FileSystem/LocalizationSystem.h"
#include "ResourcesManageHelper.h"

using namespace DAVA;

LocalizationEditorDialog::LocalizationEditorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LocalizationEditorDialog)
{
    ui->setupUi(this);
    FillLocaleComboBox();
    ConnectToSignals();
    SetLocalizationDirectoryPath();
    
    HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platformNode)
	{
    	QString platformName = platformNode ? platformNode->GetName() : "none";
    	QString windowTitle = QString("Localization settings for platform \"%1\"").arg(platformName);
    	setWindowTitle(windowTitle);
		// Enable open localization dialog button only if platfrom is available
		ui->openLocalizationFileButton->setEnabled(true);
	}
}

LocalizationEditorDialog::~LocalizationEditorDialog()
{
    delete ui;
}

void LocalizationEditorDialog::FillLocaleComboBox()
{
    // Get count of supported languages
    int languagesCount = LocalizationSystemHelper::GetSupportedLanguagesCount();
    QString languageDescription;
    // Fill combobox with language values
    for (int i = 0; i < languagesCount; ++i) {
        languageDescription =  QString::fromStdString(LocalizationSystemHelper::GetSupportedLanguageDesc(i));
        ui->currentLocaleComboBox->addItem(languageDescription);
    }
    // Setup default locale
    SetDefaultLanguage();
}

void LocalizationEditorDialog::ConnectToSignals()
{
    // Open locale directory button clicked event
    connect(ui->openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    connect(ui->currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void LocalizationEditorDialog::SetLocalizationDirectoryPath()
{
    QString defaultPath = QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath());
    if (!defaultPath.isEmpty())
    {
        ui->localizationFilePathLineEdit->setText(defaultPath);
    }
}

void LocalizationEditorDialog::SetDefaultLanguage()
{
    // Get description for current language ID
    String currentLanguageID = LocalizationSystem::Instance()->GetCurrentLocale();
    String languageDescription = LocalizationSystemHelper::GetLanguageDescByLanguageID(currentLanguageID);

    // Setup combo box value
    int index = ui->currentLocaleComboBox->findText(QString::fromStdString(languageDescription));
    ui->currentLocaleComboBox->setCurrentIndex(index);
}

void LocalizationEditorDialog::OnOpenLocalizationFileButtonClicked()
{
    QString fileDirectory = QFileDialog::getExistingDirectory(this, tr( "Select localization files directory" ),
																ResourcesManageHelper::GetDefaultDirectory());

	if(!fileDirectory.isNull() && !fileDirectory.isEmpty())
    {
		// Convert directory path into Unix-style path
		fileDirectory = ResourcesManageHelper::ConvertPathToUnixStyle(fileDirectory);

		if (ResourcesManageHelper::ValidateResourcePath(fileDirectory))
        {
			// Get localization relative path
			QString localizationRelativePath = ResourcesManageHelper::GetResourceRelativePath(fileDirectory);
      		// Update ui
      		ui->localizationFilePathLineEdit->setText(localizationRelativePath);
     		ReinitializeLocalizationSystem(localizationRelativePath);
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(fileDirectory);
		}
     }
}

void LocalizationEditorDialog::OnCurrentLocaleChanged(int /*index*/)
{
    ReinitializeLocalizationSystem(QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath()));
}

void LocalizationEditorDialog::ReinitializeLocalizationSystem(const QString& localizationDirectory)
{
    int languageItemID = this->ui->currentLocaleComboBox->currentIndex();
    if (languageItemID == -1)
    {
        return;
    }

    String languageId = LocalizationSystemHelper::GetSupportedLanguageID(languageItemID);
    
    // Re-initialize the Localization System with the new Locale.
    LocalizationSystem::Instance()->Cleanup();
    
    if (!localizationDirectory.isEmpty())
    {
        LocalizationSystem::Instance()->SetCurrentLocale(languageId);
        LocalizationSystem::Instance()->InitWithDirectory(localizationDirectory.toStdString());
    }
    
    HierarchyTreeController::Instance()->UpdateLocalization(true);
}