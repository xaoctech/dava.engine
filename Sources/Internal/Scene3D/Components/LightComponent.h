#ifndef __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Light.h"

namespace DAVA 
{

class LightComponent : public Component
{
public:
    LightComponent(Light * _light = 0);
    ~LightComponent();
    
    IMPLEMENT_COMPONENT_TYPE(LIGHT_COMPONENT);
    virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

    void SetLightObject(Light * _light);
    Light * GetLightObject();
    
private:
    Light * light;
    
public:
    
    INTROSPECTION_EXTEND(LightComponent, Component,
        MEMBER(light, "Light", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
