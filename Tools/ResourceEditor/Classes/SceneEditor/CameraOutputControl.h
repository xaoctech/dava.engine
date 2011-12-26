#ifndef __CAMERA_OUTPUT_CONTROL_H__
#define __CAMERA_OUTPUT_CONTROL_H__

#include "DAVAEngine.h"
#include "OutputControl.h"

using namespace DAVA;

class EditorScene;
class CameraOutputControl: public OutputControl
{
    
public:
    CameraOutputControl(EditorScene *_scene, const Rect & rect);
    virtual ~CameraOutputControl();

    virtual void WillAppear();
    virtual void Update(float32 timeElapsed);

    void ReadCamera(bool refreshList);

protected:

    
    EditorScene *scene;
    float32 timeAfterUpdate;
};



#endif // __CAMERA_OUTPUT_CONTROL_H__