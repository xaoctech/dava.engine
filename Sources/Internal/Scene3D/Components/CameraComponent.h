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

	Camera* GetCamera();
	void SetCamera(Camera * _camera);

    IMPLEMENT_COMPONENT_TYPE(CAMERA_COMPONENT);

    virtual Component* Clone(Entity *toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
    
private:
    Camera* camera;
    
public:
    INTROSPECTION_EXTEND(CameraComponent, Component,
        PROPERTY(camera, "Camera", GetCamera, SetCamera, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_CAMERA_COMPONENT_H__
