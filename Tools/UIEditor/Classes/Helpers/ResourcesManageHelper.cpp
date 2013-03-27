//
//  ResourcesManageHelper.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 11/19/12.
//
//

#include "ResourcesManageHelper.h"
#include "HierarchyTreeController.h"
#include "HierarchyTreePlatformNode.h"
#include "EditorSettings.h"
#include "StringUtils.h"
#include "FileSystem/FileSystem.h"

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

/* 
	Project folders structure:
	
	$project - absolute path to project directory.
	$project/Data/UI/ui.uieditor - path to project file. Each project can have only one project file.
	$project/Data/UI/ - path to platform root folder. It this folders all available platfroms are stored.
	
	$project/DataSource/ - root directory for all unpacked resources.
	$project/DataSource/Gfx/ - path to unpacked sprites (usually in PSD format).
	$project/DataSource/Gfx/Fonts/ - path to unpacked graphics fonts sprites

	$project/Data/ - root directory for all project resources (fonts, sprites etc.)
	$project/Data/Gfx/ - path to packed sprites, which can be used in project.
	$project/Data/Fonts/ - path to true type fonts.
	$project/Data/Fontdef/ - path to graphic fonts definition files.
	$project/DAta/Gfx/Fonts/ - path to packed graphic fonts sprites.
*/

// Resource folder header
static const QString RES_HEADER = "~res:";
// True type fonts resource folder path
static const String FONTS_RES_PATH = "~res:/Fonts/";
// Graphics fonts definition resource folder path
static const String GRAPHICS_FONTS_RES_PATH = "~res:/Fontdef/";
// Button background image path
static const String BACKGROUND_IMAGE_PATH = "~res:/Images/buttonBg.png";
// Help contents path
// Help contents path
#if defined(__DAVAENGINE_WIN32__)
static const String HELP_CONTENTS_PATH = "/Data/Help/UIEditor.html";
#else
static const String HELP_CONTENTS_PATH = "~res:/Help/UIEditor.html";
#endif
// Additional text constants
static const QString GFX = "/Gfx/";
static const QString FONTS = "/Fonts/";
// Project DATASOURCE folder
static const QString PROJECT_DATASOURCE = "%1/DataSource";
// Project DATA folder
static const QString PROJECT_DATA = "%1/Data";
// Platform directory path
static const QString PROJECT_PLATFORM_PATH = PROJECT_DATA + "/UI/";
// Project file path
static const QString PROJECT_FILE_PATH = "%1/ui.uieditor";
// Project GFX folder for sprites psd files
static const QString PROJECT_DATASOURCE_GFX = PROJECT_DATASOURCE + GFX;
// Project GFX folder for graphics fonts sprites psd files
static const QString PROJECT_DATASOURCE_GRAPHICS_FONTS = PROJECT_DATASOURCE_GFX + FONTS;
// Project converted sprites folder
static const QString PROJECT_DATA_GFX = PROJECT_DATA + GFX;
// Project converted graphics fonts sprites folder
static const QString PROJECT_DATA_GRAPHICS_FONTS = PROJECT_DATA_GFX + FONTS;

// Resource wrong location error message
static const QString RES_WRONG_LOCATION_ERROR_MESSAGE = "Resource %1 is not located inside project 'Data' folder. It can't be linked with project or control!";

//Available fonts extensions
static const QStringList FONTS_EXTENSIONS_FILTER = (QStringList() << "*.ttf" << "*.otf" << "*.fon" << "*.fnt" << "*.def");

QString ResourcesManageHelper::buttonBackgroundImagePath;
QString ResourcesManageHelper::helpContentsPath;
QString ResourcesManageHelper::projectTitle;

QString ResourcesManageHelper::GetFontAbsolutePath(const QString& resourceFileName, bool graphicsFont)
{	
    QString fontPath = graphicsFont ? QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(GRAPHICS_FONTS_RES_PATH))
									: QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(FONTS_RES_PATH));
    fontPath += resourceFileName;
	
    return fontPath;    
}

QString ResourcesManageHelper::GetFontRelativePath(const QString& resourceFileName, bool graphicsFont)
{
    QString fontPath = graphicsFont ? QString::fromStdString(GRAPHICS_FONTS_RES_PATH)
									: QString::fromStdString(FONTS_RES_PATH);
    fontPath += resourceFileName;
	
    return fontPath;
}

bool ResourcesManageHelper::ValidateResourcePath(const QString& resourcePath)
{
	const QString& resourceFolder = GetResourceRootDirectory();
	// Check if given resource is located inside resource folder
	return resourcePath.contains(resourceFolder);
}

QString ResourcesManageHelper::GetResourceRelativePath(const QString& resourceAbsolutePath, bool keepFileExtension)
{
    QFileInfo fileInfo(resourceAbsolutePath);
    QString processedResourcePath = resourceAbsolutePath;
    
    if (fileInfo.exists())
    {
        //We should keep file extension if needed
        if (keepFileExtension)
            processedResourcePath = resourceAbsolutePath;
        else
        {
            //Remove file extension
            QString resourceExtension = QString(".%1").arg(fileInfo.suffix());
            processedResourcePath = TruncateFileExtension(resourceAbsolutePath, resourceExtension);
        }
		//Convert absolute path to relative one, if possible.
        const QString& resourceFolder = GetResourceRootDirectory();
        if (processedResourcePath.startsWith(resourceFolder, Qt::CaseInsensitive))
        {
        	processedResourcePath.replace(resourceFolder, RES_HEADER, Qt::CaseInsensitive);
		}
    }

    return processedResourcePath;
}

QStringList ResourcesManageHelper::GetFontsList()
{
    QStringList filesNamesList;
	// Get true type fonts
    // Get absoulute path
    QString fontsPath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(FONTS_RES_PATH));
    QDir dir(fontsPath);
    // Get the list of files in fonts directory
    filesNamesList = dir.entryList(FONTS_EXTENSIONS_FILTER, QDir::Files);	
	fontsPath.clear();
	
	// Get graphics fonts
	fontsPath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(GRAPHICS_FONTS_RES_PATH));
	// If we can't open graphics font directory - do nothing
	if (dir.cd(fontsPath))
	{
		filesNamesList.append(dir.entryList(FONTS_EXTENSIONS_FILTER, QDir::Files));
    }
    return filesNamesList;
}

void ResourcesManageHelper::InitInternalResources()
{
	buttonBackgroundImagePath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(BACKGROUND_IMAGE_PATH));

#if defined(__DAVAENGINE_WIN32__)
	String currentFolder = FileSystem::Instance()->GetCurrentWorkingDirectory();
	helpContentsPath = ConvertPathToUnixStyle(QString::fromStdString(currentFolder + HELP_CONTENTS_PATH));
#else
    helpContentsPath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(HELP_CONTENTS_PATH));
#endif
	// Save project default title
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
           	projectTitle = options->GetString("title", "UIEditor").c_str();
        }
    }
	// If project name wasn't set - create default name
	if (projectTitle.isNull() || projectTitle.isEmpty())
		projectTitle = "UIEditor";
}

QString ResourcesManageHelper::GetButtonBackgroundImagePath()
{
	return buttonBackgroundImagePath;
}

QString ResourcesManageHelper::GetHelpContentsPath()
{
	return helpContentsPath;
}

QString ResourcesManageHelper::GetProjectPath()
{
	const HierarchyTreeRootNode *rootNode = HierarchyTreeController::Instance()->GetTree().GetRootNode();
	if (!rootNode)
		return QString();
	
	return rootNode->GetProjectDir();
}

QString ResourcesManageHelper::GetProjectTitle()
{
	// Set default project title
	QString projectTitleString = projectTitle;
	// Get active project path
	QString projectPath = GetProjectPath();
	if (!projectPath.isNull() && !projectPath.isEmpty())
	{
		QString projectFilePath = GetProjectFilePath(projectPath);
		projectTitleString = QString("%1 - %2").arg(projectTitle).arg(projectFilePath);
	}
	return projectTitleString;
}

QString ResourcesManageHelper::GetProjectTitle(const QString& projectFilePath)
{
	return QString("%1 - %2").arg(projectTitle).arg(projectFilePath);
}

QString ResourcesManageHelper::GetDefaultDirectory()
{
	QString defaultDir = QString::fromStdString(EditorSettings::Instance()->GetProjectPath());
	//If default directory path is not available in project settings - use current working path
	if ( defaultDir.isNull() || defaultDir.isEmpty() )
	{
  		defaultDir = QDir::currentPath();
	}
	return defaultDir;
}

QString ResourcesManageHelper::GetResourceRootDirectory()
{
	QString projectPath = GetProjectPath();
	if (projectPath.isNull() || projectPath.isEmpty())
	{
		return QString();
	}
	return GetDataPath(projectPath);
}

QString ResourcesManageHelper::GetResourceFolder(const QString& resourcePath)
{
	QString projectPath = GetProjectPath();
	if (projectPath.isNull() || projectPath.isEmpty())
	{
		return QString();
	}
	return QString(resourcePath).arg(projectPath);
}

QString ResourcesManageHelper::GetSpritesDirectory()
{
	return GetResourceFolder(PROJECT_DATA_GFX);
}

QString ResourcesManageHelper::GetFontSpritesDirectory()
{
	return GetResourceFolder(PROJECT_DATA_GRAPHICS_FONTS);
}

QString ResourcesManageHelper::GetSpritesDatasourceDirectory()
{
	return GetResourceFolder(PROJECT_DATASOURCE_GFX);
}

QString ResourcesManageHelper::GetFontSpritesDatasourceDirectory()
{
	return GetResourceFolder(PROJECT_DATASOURCE_GRAPHICS_FONTS);
}

QString ResourcesManageHelper::GetDataPath(const QString& projectPath)
{
	return QString(PROJECT_DATA).arg(projectPath);
}

QString ResourcesManageHelper::GetPlatformRootPath(const QString& projectPath)
{
	return QString(PROJECT_PLATFORM_PATH).arg(projectPath);
}
	
QString ResourcesManageHelper::GetProjectFilePath(const QString& projectPath)
{	
	return QString(PROJECT_FILE_PATH).arg(projectPath);
}

void ResourcesManageHelper::ShowErrorMessage(const QString& messageParam)
{
	QMessageBox messageBox;
	messageBox.setText(QString(RES_WRONG_LOCATION_ERROR_MESSAGE).arg(messageParam));
	messageBox.setStandardButtons(QMessageBox::Ok);
	messageBox.exec();
}

QString ResourcesManageHelper::ConvertPathToUnixStyle(const QString& inputString)
{
	// Replace simple slash to unix style slash
	QString outputString = inputString;
	outputString.replace(QString("\\") ,QString("/"));
	return outputString;
}