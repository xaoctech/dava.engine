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
    static QString GetFontRelativePath(const QString& resourceFileName);
    static QString GetFontAbsolutePath(const QString& resourceFileName);
    static QString GetResourceRelativePath(const QString& resourceAbsolutePath, bool keepFileExtension = false);
    static QStringList GetFontsList();
	// Validate if selected resource is located inside project 
	static bool ValidateResourcePath(const QString& resourcePath);
	// Functions to work with help contents and button background image
	static QString GetButtonBackgroundImagePath();
	static QString GetHelpContentsPath();
	// Working directory
	static QString GetDefaultDirectory();
	// Resource directory
	static QString GetResourceDirectory();
	// Get project title string
	static QString GetProjectTitle();
	// Path to palform specific files
	static QString GetPlatformPath(const QString& projectPath);
	// Project file path
	static QString GetProjectFilePath(const QString& projectPath);
	// Initialize project internal resources
	// Do not use this function elsewhere than during application init!!!
	static void InitInternalResources();
	// Show error message
	static void ShowErrorMessage(const QString& messageParam);
private:
	static QString buttonBackgroundImagePath;
	static QString helpContentsPath;
	static QString projectTitle;
};
};

#endif