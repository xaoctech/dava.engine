#ifndef __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_DEBUG_RENDER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA 
{

class DebugRenderComponent : public Component
{
public:
    enum
	{
		DEBUG_DRAW_NONE			= 0x0,
		DEBUG_DRAW_AABBOX		= 0x1,
		DEBUG_DRAW_LOCAL_AXIS	= 0x2,
		DEBUG_DRAW_AABOX_CORNERS= 0x4,
		DEBUG_DRAW_LIGHT_NODE	= 0x8,
        DEBUG_DRAW_NORMALS		= 0x10,
        DEBUG_DRAW_GRID			= 0x20,
		DEBUG_DRAW_USERNODE		= 0x40,
		DEBUG_DRAW_RED_AABBOX	= 0x80,
		DEBUG_DRAW_CAMERA		= 0x100,

		DEBUG_AUTOCREATED		= 0x80000000,
        DEBUG_DRAW_ALL			= 0xFFFFFFFF,
	};
    
    DebugRenderComponent();
    virtual ~DebugRenderComponent();
    
    IMPLEMENT_COMPONENT_TYPE(DEBUG_RENDER_COMPONENT);

    void SetDebugFlags(uint32 debugFlags);
    uint32 GetDebugFlags();

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

private:
    uint32 curDebugFlags;
    
public:
    INTROSPECTION_EXTEND(DebugRenderComponent, Component,
        PROPERTY("curDebugFlags", "Debug Flags ", GetDebugFlags, SetDebugFlags, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};


};

#endif //__DAVAENGINE_SCENE3D_RENDER_COMPONENT_H__
