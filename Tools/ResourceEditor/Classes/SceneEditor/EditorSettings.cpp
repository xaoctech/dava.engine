#include "EditorSettings.h"

EditorSettings::EditorSettings()
{
    settings = new KeyedArchive();
    
    settings->Load("~doc:/ResourceEditorOptions.archive");
}
    
EditorSettings::~EditorSettings()
{
    SafeRelease(settings);
}


KeyedArchive *EditorSettings::GetSettings()
{
    return settings;
}


void EditorSettings::Save()
{
    settings->Save("~doc:/ResourceEditorOptions.archive");
}

String EditorSettings::GetDataSourcePath()
{
    return settings->GetString("3dDataSourcePath", "/");
}

bool EditorSettings::IsValidPath(const String &path)
{
    if(path.length() == 0) //for texture resetting
        return true;
    
    size_t posPng = path.find(".png");
    size_t posPvr = path.find(".pvr");
    return ((String::npos != posPng) || (String::npos != posPvr));
}

float32 EditorSettings::GetCameraSpeed()
{
    static const float32 speedConst[] = {35, 100, 250, 400};
    return speedConst[settings->GetInt32("CameraSpeed", 0)];
}
void EditorSettings::SetCameraSpeedIndex(int32 camSpeedIndex)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
    settings->SetInt32("CameraSpeed", camSpeedIndex);
    Save();
}

//String EditorSettings::GetProjectPath()
//{
//    String projectPath = settings->GetString("ProjectPath", "/");
//    if('/' != projectPath[projectPath.length() - 1])
//    {
//        projectPath += '/';
//    }
// 
//    return projectPath;
//}