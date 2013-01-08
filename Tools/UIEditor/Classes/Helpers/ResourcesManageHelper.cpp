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

// Resource folder header
static const QString RES_HEADER = "~res:";
// Font resource folder path
static const String FONTS_RES_PATH = "~res:/Fonts/";
// Button background image path
static const String BACKGROUND_IMAGE_PATH = "~res:/Images/buttonBg.png";
// Help contents path
#if defined(__DAVAENGINE_WIN32__)
static const String HELP_CONTENTS_PATH = "/Data/Help/UIEditor.html";
#else
static const String HELP_CONTENTS_PATH = "~res:/Help/UIEditor.html";
#endif

// Platform directory path
static const QString PROJECT_PLATFORM_PATH = "%1/DataSource/UI/";
// Project file path
static const QString PROJECT_FILE_NAME = PROJECT_PLATFORM_PATH + "ui.uieditor";
// Resource wrong location error message
static const QString RES_WRONG_LOCATION_ERROR_MESSAGE = "File %1 is not located inside platform resource folder. It can't be linked with control!";

//Available fonts extensions
static const QStringList FONTS_EXTENSIONS_FILTER = (QStringList() << "*.ttf" << "*.otf" << "*.fon" << "*.fnt" << "*.def");

QString ResourcesManageHelper::buttonBackgroundImagePath;
QString ResourcesManageHelper::helpContentsPath;
QString ResourcesManageHelper::projectTitle;

QString ResourcesManageHelper::GetFontAbsolutePath(const QString& resourceFileName)
{
    QString fontPath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(FONTS_RES_PATH));
    fontPath += resourceFileName;
    return fontPath;    
}

QString ResourcesManageHelper::GetFontRelativePath(const QString& resourceFileName)
{
    QString fontPath = QString::fromStdString(FONTS_RES_PATH);
    fontPath += resourceFileName;
    return fontPath;
}

bool ResourcesManageHelper::ValidateResourcePath(const QString& resourcePath)
{
	HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
    if (!platformNode)
		return false;		
	
	const QString& resourceFolder = platformNode->GetResourceFolder();
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
        HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
        if (platformNode)
        {
            const QString& resourceFolder = platformNode->GetResourceFolder();
            if (processedResourcePath.startsWith(resourceFolder, Qt::CaseInsensitive))
            {
                processedResourcePath.replace(resourceFolder, RES_HEADER, Qt::CaseInsensitive);
            }
        }
    }

    return processedResourcePath;
}

QStringList ResourcesManageHelper::GetFontsList()
{
    QStringList filesNamesList;
    //Get absoulute path
    QString fontsPath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(FONTS_RES_PATH));
    QDir dir(fontsPath);
    //Get the list of files in fonts directory
    filesNamesList = dir.entryList(FONTS_EXTENSIONS_FILTER, QDir::Files);
    
    return filesNamesList;
}

void ResourcesManageHelper::InitInternalResources()
{
	buttonBackgroundImagePath = QString::fromStdString(FileSystem::Instance()->SystemPathForFrameworkPath(BACKGROUND_IMAGE_PATH));

#if defined(__DAVAENGINE_WIN32__)
	String currentFolder = FileSystem::Instance()->GetCurrentWorkingDirectory();
	helpContentsPath = QString::fromStdString(currentFolder + HELP_CONTENTS_PATH);
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

QString ResourcesManageHelper::GetProjectTitle()
{
	QString projectTitleString = projectTitle;	
	HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
	if (platformNode)
	{
		HierarchyTreeRootNode *rootNode = (HierarchyTreeRootNode *)platformNode->GetRoot();
		if (rootNode)
		{
			projectTitleString = QString("%1 - %2").arg(projectTitle).arg(rootNode->GetProjectPath());
		}		
	}
	return projectTitleString;
}

QString ResourcesManageHelper::GetResourceDirectory()
{
	HierarchyTreePlatformNode* platformNode = HierarchyTreeController::Instance()->GetActivePlatform();
	if (!platformNode)
		return QString();
		
	const QString resourceDir = platformNode->GetResourceFolder();
	return resourceDir;
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

void ResourcesManageHelper::ShowErrorMessage(const QString& messageParam)
{
	QMessageBox messageBox;
	messageBox.setText(QString(RES_WRONG_LOCATION_ERROR_MESSAGE).arg(messageParam));
	messageBox.setStandardButtons(QMessageBox::Ok);
	messageBox.exec();
}

QString ResourcesManageHelper::GetPlatformPath(const QString& projectPath)
{
	return QString(PROJECT_PLATFORM_PATH).arg(projectPath);
}
	
QString ResourcesManageHelper::GetProjectFilePath(const QString& projectPath)
{
	return QString(PROJECT_FILE_NAME).arg(projectPath);
}