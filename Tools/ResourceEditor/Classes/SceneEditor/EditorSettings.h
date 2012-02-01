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

    int32 GetLodLayersCount();
    float32 GetLodLayerNear(int32 layerNum);
    float32 GetLodLayerFar(int32 layerNum);

    void SetLodLayersCount(int32 count);
    void SetLodLayerNear(int32 layerNum, float32 near);
    void SetLodLayerFar(int32 layerNum, float32 far);

    int32 GetForceLodLayer();
    void SetForceLodLayer(int32 layer);

    
protected:

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__