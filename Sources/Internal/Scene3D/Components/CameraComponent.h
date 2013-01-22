#ifndef __DAVAENGINE_CAMERA_COMPONENT_H__
#define __DAVAENGINE_CAMERA_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA 
{

class Camera;

class CameraComponent : public Component
{
public:
    CameraComponent(Camera * _camera = 0);
    virtual ~CameraComponent();

    IMPLEMENT_COMPONENT_TYPE(CAMERA_COMPONENT);

    virtual Component * Clone();

    void SetCamera(Camera * _camera);
    
private:
    Camera * camera;
    
public:

    INTROSPECTION_EXTEND(CameraComponent, Component,
        MEMBER(camera, "Camera", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );

};


};

#endif //__DAVAENGINE_CAMERA_COMPONENT_H__
