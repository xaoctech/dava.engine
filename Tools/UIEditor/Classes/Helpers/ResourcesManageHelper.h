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
	// Functions to work with help contents and button background image
	static QString GetButtonBackgroundImagePath();
	static QString GetHelpContentsPath();
	// Working directory
	static QString GetDefaultDirectory();
	// Initialize project internal resources
	// Do not use this function elsewhere than during application init!!!
	static void InitInternalResources();
private:
	static QString buttonBackgroundImagePath;
	static QString helpContentsPath;
};
};

#endif