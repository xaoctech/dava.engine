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

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include "StringUtils.h"
#include "FileSystem/FileSystem.h"
#include <QDir>

//Resource folder header
static const QString RES_HEADER = "~res:/";
//Font resource folder path
static const String FONTS_RES_PATH = "~res:/Fonts/";
//Button background image path
static const String BACKGROUND_IMAGE_PATH = "~res:/Images/buttonBg.png";

//Help contents path
#if defined(__DAVAENGINE_WIN32__)
static const String HELP_CONTENTS_PATH = "/Data/Help/UIEditor.html";
#else
static const String HELP_CONTENTS_PATH = "~res:/Help/UIEditor.html";
#endif

//Available fonts extensions
static const QStringList FONTS_EXTENSIONS_FILTER = (QStringList() << "*.ttf" << "*.otf" << "*.fon" << "*.fnt" << "*.def");

QString ResourcesManageHelper::buttonBackgroundImagePath;
QString ResourcesManageHelper::helpContentsPath;

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
}

QString ResourcesManageHelper::GetButtonBackgroundImagePath()
{
	return buttonBackgroundImagePath;
}

QString ResourcesManageHelper::GetHelpContentsPath()
{
	return helpContentsPath;
}