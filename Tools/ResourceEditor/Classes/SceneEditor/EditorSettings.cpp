#include "EditorSettings.h"

#include "ControlsFactory.h"

#include "Scene3D/Heightmap.h"


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
    size_t posHeightmap = path.find(Heightmap::FileExtension());
    return ((String::npos != posPng) || (String::npos != posPvr) || (String::npos != posHeightmap));
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

float32 EditorSettings::GetLodLayerDistance(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < LodNode::MAX_LOD_LAYERS);
    return settings->GetFloat(Format("LODLayer_%d", layerNum), LodNode::GetDefaultDistance(layerNum));
}

void EditorSettings::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < LodNode::MAX_LOD_LAYERS);
    return settings->SetFloat(Format("LODLayer_%d", layerNum), distance);
}

int32 EditorSettings::GetLastOpenedCount()
{
    return settings->GetInt32("LastOpenedFilesCount", 0);
}

String EditorSettings::GetLastOpenedFile(int32 index)
{
    int32 count = GetLastOpenedCount();
    DVASSERT((0 <= index) && (index < count));
    
    return settings->GetString(Format("LastOpenedFile_%d", index), "");
}

void EditorSettings::AddLastOpenedFile(const String & pathToFile)
{
    Vector<String> filesList;
    
    int32 count = GetLastOpenedCount();
    for(int32 i = 0; i < count; ++i)
    {
        String path = settings->GetString(Format("LastOpenedFile_%d", i), "");
        if(path == pathToFile)
        {
            return;
        }
        
        filesList.push_back(path);
    }

    
    filesList.insert(filesList.begin(), pathToFile);
    count = 0;
    for(;(count < filesList.size()) && (count < RESENT_FILES_COUNT); ++count)
    {
        settings->SetString(Format("LastOpenedFile_%d", count), filesList[count]);
    }
    settings->SetInt32("LastOpenedFilesCount", count);
    
    Save();
}

void EditorSettings::SetDrawGrid(bool drawGrid)
{
    settings->SetBool("DrawGrid", drawGrid);
}

bool EditorSettings::GetDrawGrid()
{
    return settings->GetBool("DrawGrid", true);
}


