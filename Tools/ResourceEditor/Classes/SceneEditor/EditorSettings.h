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
    
protected:

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__