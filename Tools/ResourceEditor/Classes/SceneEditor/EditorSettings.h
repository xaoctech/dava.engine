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

	void ApplyOptions();

    void SetProjectPath(const FilePath &projectPath);
    FilePath GetProjectPath();
    
    void SetDataSourcePath(const FilePath &datasourcePath);
    FilePath GetDataSourcePath();
    
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
    void AddLastOpenedFile(const FilePath & pathToFile);
    
    void SetDrawGrid(bool drawGrid);
    bool GetDrawGrid();

	void SetEnableImposters(bool enableImposters);
	bool GetEnableImposters();
    
    eGPUFamily GetTextureViewGPU();
    void SetTextureViewGPU(int32 gpu);

    
	void SetMaterialsColor(const Color &ambient, const Color &diffuse, const Color &specular);
	Color GetMaterialAmbientColor();
	Color GetMaterialDiffuseColor();
	Color GetMaterialSpecularColor();

	FilePath GetParticlesConfigsPath();
    
protected:

	Vector4 ToVector4(const Color &color);
	Color ToColor(const Vector4 &colorVector);

    KeyedArchive *settings;
};



#endif // __EDITOR_SETTINGS_H__