//
//  EditorSettings.h
//  UIEditor
//
//  Created by Denis Bespalov on 12/20/12.
//
//

#ifndef UIEditor_EditorSettings_h
#define UIEditor_EditorSettings_h

#include "DAVAEngine.h"

using namespace DAVA;

class EditorSettings: public Singleton<EditorSettings>
{

public: 
    enum eDefaultSettings
    {
        RECENT_FILES_COUNT = 5,
    };
	
public:
	EditorSettings();
    virtual ~EditorSettings();

    KeyedArchive *GetSettings();
    void Save();
	
	void SetProjectPath(const String &projectPath);
    String GetProjectPath();
	
    int32 GetLastOpenedCount();
    String GetLastOpenedFile(int32 index);
    void AddLastOpenedFile(const String & pathToFile);

protected:
	KeyedArchive *settings;
	
};

#endif //UIEditor_EditorSettings_h
