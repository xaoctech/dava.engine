#ifndef __SCENE_PREVIEW_CONTROL_H__
#define __SCENE_PREVIEW_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "../EditorScene.h"

using namespace DAVA;

class PreviewCameraController: public CameraController
{
public:
    
    PreviewCameraController();
    
    virtual void Input(UIEvent * event);
    virtual void SetCamera(Camera * camera);
    
    void SetRadius(float32 _radius);
    void UpdateCamera();
    
    void SetControlHeight(int32 height);
    
private:

    
    float32 angleVertical;
    float32 angleHorizontal;
    
    float32 radius;
    
    Vector2 moveStartPt;
    Vector2 moveStopPt;
    
    Vector2 zoomStartPt;
    Vector2 zoomStopPt;
    
    float32 zoomLevel;
    
    int32 controlHeight;
};

class ScenePreviewControl: public UI3DView
{
    
public:
    ScenePreviewControl(const Rect & rect);
    virtual ~ScenePreviewControl();
    
    virtual void Input(UIEvent * touch);
    virtual void Update(float32 timeElapsed);

    void OpenScene(const String &pathToFile);
    
protected:

    void SetupCamera();
    
    //scene controls
    EditorScene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
    PreviewCameraController * cameraController;
    
    String currentScenePath;
    SceneNode *rootNode;
    
    bool needSetCamera;
    bool sceCamera;
};



#endif // __SCENE_PREVIEW_CONTROL_H__