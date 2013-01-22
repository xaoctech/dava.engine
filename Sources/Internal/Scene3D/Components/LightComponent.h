#ifndef __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/Highlevel/Light.h"

namespace DAVA 
{

class LightComponent : public Component
{
public:
    LightComponent(LightNode * _light = 0);
    ~LightComponent();
    
    IMPLEMENT_COMPONENT_TYPE(LIGHT_COMPONENT);
    virtual Component * Clone();

    void SetLightObject(LightNode * _light);
    LightNode * GetLightObject();
    
private:
    LightNode * light;
    
public:
    
    INTROSPECTION_EXTEND(LightComponent, Component,
        MEMBER(light, "Light", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_LIGHT_COMPONENT_H__
