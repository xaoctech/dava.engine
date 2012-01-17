#ifndef __EDITOR_SETTINGS_H__
#define __EDITOR_SETTINGS_H__

#include "DAVAEngine.h"

using namespace DAVA;

class EditorSettings: public Singleton<EditorSettings>
{
    
public:
    EditorSettings();
    virtual ~EditorSettings();

    KeyedArchive *GetSettings();
    void Save();

//    String GetProjectPath();
    String GetDataSourcePath();
    
    static bool IsValidPath(const String &path);

    float32 GetCameraSpeed();
    void SetCameraSpeedIndex(int32 camSpeedIndex);//0 - 4

protected:

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__