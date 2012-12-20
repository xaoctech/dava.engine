#ifndef __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Components/Component.h"
#include "Scene3D/Render/RenderObject.h"

namespace DAVA 
{

class RenderComponent : public Component
{
public:
    RenderComponent();
    ~RenderComponent();
    
    IMPLEMENT_COMPONENT_TYPE(RENDER_COMPONENT);
    Component * Clone();

    void SetRenderObject(RenderObject * object);
    RenderObject * GetRenderObject();
private:
    RenderObject * renderObject;
};


};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
