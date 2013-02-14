#ifndef __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/SceneNode.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA 
{

class DebugRenderComponent : public Component
{
public:
    enum
	{
		DEBUG_DRAW_NONE = 0,
		DEBUG_DRAW_AABBOX = 1,
		DEBUG_DRAW_LOCAL_AXIS = 2,
		DEBUG_DRAW_AABOX_CORNERS = 4,
		DEBUG_DRAW_LIGHT_NODE = 8,
        DEBUG_DRAW_NORMALS = 16,
        DEBUG_DRAW_GRID = 32,
		DEBUG_DRAW_USERNODE = 64,
		DEBUG_DRAW_RED_AABBOX = 128,
        DEBUG_DRAW_ALL = 0xFFFFFFFF,
	};
    
    DebugRenderComponent();
    virtual ~DebugRenderComponent();
    
    IMPLEMENT_COMPONENT_TYPE(DEBUG_RENDER_COMPONENT);

    void SetDebugFlags(uint32 debugFlags);
    uint32 GetDebugFlags();

	virtual Component * Clone(SceneNode * toEntity);
	virtual void Serialize(KeyedArchive *archive);
	virtual void Deserialize(KeyedArchive *archive);

private:
    uint32 debugFlags;
    
public:
    INTROSPECTION_EXTEND(DebugRenderComponent, Component,
        PROPERTY(debugFlags, "Debug Flags ", GetDebugFlags, SetDebugFlags, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
