#include "EditorSettings.h"

#include "ControlsFactory.h"

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
    int32 index = settings->GetInt32("CameraSpeedIndex", 0);
    return GetCameraSpeed(index);
}

void EditorSettings::SetCameraSpeedIndex(int32 camSpeedIndex)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);

    settings->SetInt32("CameraSpeedIndex", camSpeedIndex);
    Save();
}

void EditorSettings::SetCameraSpeed(int32 camSpeedIndex, float32 speed)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
    settings->SetFloat("CameraSpeedValue" + camSpeedIndex, speed);
}

float32 EditorSettings::GetCameraSpeed(int32 camSpeedIndex)
{
    DVASSERT(camSpeedIndex >= 0 && camSpeedIndex < 4);
    
    static const float32 speedConst[] = {35, 100, 250, 400};
    return settings->GetFloat("CameraSpeedValue" + camSpeedIndex, speedConst[camSpeedIndex]);
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

int32 EditorSettings::GetScreenWidth()
{
    return settings->GetInt32("ScreenWidth", 1024);
}

void EditorSettings::SetScreenWidth(int32 width)
{
    settings->SetInt32("ScreenWidth", width);
}

int32 EditorSettings::GetScreenHeight()
{
    return settings->GetInt32("ScreenHeight", 690);
}

void EditorSettings::SetScreenHeight(int32 height)
{
    settings->SetInt32("ScreenHeight", height);
}

float32 EditorSettings::GetAutosaveTime()
{
    return settings->GetFloat("AutoSaveTime", 5.0f);
}
void EditorSettings::SetAutosaveTime(float32 time)
{
    settings->SetFloat("AutoSaveTime", time);
}

String EditorSettings::GetLanguage()
{
    return settings->GetString("Language", "en");
}

void EditorSettings::SetLanguage(const String &language)
{
    settings->SetString("Language", language);
}

bool EditorSettings::GetShowOutput()
{
    return settings->GetBool("ShowOutput", true);
}

void EditorSettings::SetShowOuput(bool showOutput)
{
    settings->SetBool("ShowOutput", showOutput);
}

int32 EditorSettings::GetLeftPanelWidth()
{
    return settings->GetInt32("LeftPanelWidth", ControlsFactory::LEFT_PANEL_WIDTH);
}

void EditorSettings::SetLeftPanelWidth(int32 width)
{
    settings->SetInt32("LeftPanelWidth", width);
}

int32 EditorSettings::GetRightPanelWidth()
{
    return settings->GetInt32("RightPanelWidth", ControlsFactory::RIGHT_PANEL_WIDTH);
}
void EditorSettings::SetRightPanelWidth(int32 width)
{
    settings->SetInt32("RightPanelWidth", width);
}

int32 EditorSettings::GetLodLayersCount()
{
    return settings->GetInt32("LODLayers_count", 8);
}

float32 EditorSettings::GetLodLayerNear(int32 layerNum)
{
    int32 count = GetLodLayersCount();
    DVASSERT(0 <= layerNum && layerNum < count);

    static const float32 LodLayerNear[] = {0, 7, 18, 990, 1990, 2990, 3990, 4990};
    return settings->GetFloat(Format("LODLayer_%d_Near", layerNum), LodLayerNear[layerNum]);
}

float32 EditorSettings::GetLodLayerFar(int32 layerNum)
{
    int32 count = GetLodLayersCount();
    DVASSERT(0 <= layerNum && layerNum < count);
    
    static const float32 LodLayerFar[] = {10, 20, 1000, 2000, 3000, 4000, 5000, 6000};
    return settings->GetFloat(Format("LODLayer_%d_Far", layerNum), LodLayerFar[layerNum]);
}

void EditorSettings::SetLodLayersCount(int32 count)
{
    settings->SetInt32("LODLayers_count", count);
}

void EditorSettings::SetLodLayerNear(int32 layerNum, float32 near)
{
    int32 count = GetLodLayersCount();
    DVASSERT(0 <= layerNum && layerNum < count);
    
    settings->SetFloat(Format("LODLayer_%d_Near", layerNum), near);
}

void EditorSettings::SetLodLayerFar(int32 layerNum, float32 far)
{
    int32 count = GetLodLayersCount();
    DVASSERT(0 <= layerNum && layerNum < count);
    
    settings->SetFloat(Format("LODLayer_%d_Far", layerNum), far);
}

int32 EditorSettings::GetForceLodLayer()
{
    return settings->GetInt32("LODLayer_Force", -1);
}

void EditorSettings::SetForceLodLayer(int32 layer)
{
    int32 count = GetLodLayersCount();
    DVASSERT(-1 <= layer && layer < count);
    
    settings->SetInt32("LODLayer_Force", layer);
}

