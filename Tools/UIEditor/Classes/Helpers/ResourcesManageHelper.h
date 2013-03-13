//
//  ResourcesManageHelper.h
//  UIEditor
//
//  Created by Denis Bespalov on 11/19/12.
//
//

#ifndef UIEditor_ResourcesManageHelper_h
#define UIEditor_ResourcesManageHelper_h

#include "Base/BaseTypes.h"

class QString;
class QStringList;

namespace DAVA
{

class ResourcesManageHelper{

public:
    // Helper to work with resources
    static QString GetFontRelativePath(const QString& resourceFileName, bool graphicsFont = false);
    static QString GetFontAbsolutePath(const QString& resourceFileName, bool graphicsFont = false);
    static QString GetResourceRelativePath(const QString& resourceAbsolutePath, bool keepFileExtension = false);
    static QStringList GetFontsList();
	// Validate if selected resource is located inside project 
	static bool ValidateResourcePath(const QString& resourcePath);
	// Functions to work with help contents and button background image
	static QString GetButtonBackgroundImagePath();
	// Get path to help contents hmtl file
	static QString GetHelpContentsPath();
	
	// Working directory
	static QString GetDefaultDirectory();
	// Resource directory
	static QString GetResourceRootDirectory();
	// Get sprites data directory
	static QString GetSpritesDirectory();
	// Get font sprites data directory
	static QString GetFontSpritesDirectory();
	// Get sprites datasource directory
	static QString GetSpritesDatasourceDirectory();
	// Get font sprites datasource directory
	static QString GetFontSpritesDatasourceDirectory();
	// Validate resource folder path
	static QString GetResourceFolder(const QString& resourcePath);
	
	// Get project title string
	static QString GetProjectTitle();
	// Get project title string
	static QString GetProjectTitle(const QString& projectFilePath);
	// Get the folder wehere project is located
	static QString GetProjectPath();
	// Get folder with resource data for specified project path
	static QString GetDataPath(const QString& projectPath);
	// Path to palform specific files for specified project path
	static QString GetPlatformRootPath(const QString& projectPath);
	// Project file path for specified project path
	static QString GetProjectFilePath(const QString& projectPath);
	// Initialize project internal resources
	// Do not use this function elsewhere than during application init!!!
	static void InitInternalResources();
	// Show error message
	static void ShowErrorMessage(const QString& messageParam);
	// Reverse slash order in string
	static QString ConvertPathToUnixStyle(const QString& inputString);

private:
	static QString buttonBackgroundImagePath;
	static QString helpContentsPath;
	static QString projectTitle;
};
};

#endif