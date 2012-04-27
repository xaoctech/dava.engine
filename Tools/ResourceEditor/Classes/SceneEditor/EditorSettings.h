#ifndef __EDITOR_SETTINGS_H__
#define __EDITOR_SETTINGS_H__

#include "DAVAEngine.h"

using namespace DAVA;

class EditorSettings: public Singleton<EditorSettings>
{
    
public: 
    
    enum eDefaultSettings
    {
        LOD_LEVELS_COUNT = 8,
        RESENT_FILES_COUNT = 5,
    };
    
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
    void SetCameraSpeed(int32 camSpeedIndex, float32 speed);
    float32 GetCameraSpeed(int32 camSpeedIndex);
    
    int32 GetScreenWidth();
    void SetScreenWidth(int32 width);
    
    int32 GetScreenHeight();
    void SetScreenHeight(int32 height);
    
    float32 GetAutosaveTime();
    void SetAutosaveTime(float32 time);
    
    String GetLanguage();
    void SetLanguage(const String &language);
    
    bool GetShowOutput();
    void SetShowOuput(bool showOutput);
    
    int32 GetLeftPanelWidth();
    void SetLeftPanelWidth(int32 width);

    int32 GetRightPanelWidth();
    void SetRightPanelWidth(int32 width);

    float32 GetLodLayerDistance(int32 layerNum);
    void SetLodLayerDistance(int32 layerNum, float32 distance);

    int32 GetLastOpenedCount();
    String GetLastOpenedFile(int32 index);
    void AddLastOpenedFile(const String & pathToFile);
    
    void SetDrawGrid(bool drawGrid);
    bool GetDrawGrid();
    
protected:

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__