#include "Scene3D/Components/CameraComponent.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"

namespace DAVA
{
    
CameraComponent::CameraComponent(Camera * _camera)
{
    camera = SafeRetain(_camera);
}

CameraComponent::~CameraComponent()
{
    SafeRelease(camera);
}

Camera* CameraComponent::GetCamera()
{
	return camera;
}
    
void CameraComponent::SetCamera(Camera * _camera)
{
	SafeRelease(camera);
    camera = SafeRetain(_camera);
}

Component* CameraComponent::Clone(Entity * toEntity)
{
    CameraComponent * newComponent = new CameraComponent();
	newComponent->SetEntity(toEntity);
    newComponent->camera = (Camera*)camera->Clone();

    return newComponent;
}

void CameraComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	Component::Serialize(archive, sceneFile);

	if(NULL != archive && NULL != camera)
	{
		KeyedArchive *camArch = new KeyedArchive();
		camera->Save(camArch);

		archive->SetArchive("cc.camera", camArch);

		camArch->Release();
	}
}

void CameraComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
{
	if(NULL != archive)
	{
		KeyedArchive *camArch = archive->GetArchive("cc.camera");
		if(NULL != camArch)
		{
			Camera* cam = new Camera();
			cam->Load(camArch);
			SetCamera(cam);
			cam->Release();
		}
	}

	Component::Deserialize(archive, sceneFile);
}
    
};