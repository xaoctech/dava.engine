//
//  SceneNodePropertyNames.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/28/12.
//
//

#ifndef ResourceEditorQt_SceneNodePropertyNames_h
#define ResourceEditorQt_SceneNodePropertyNames_h

#include "Scene3D/Entity.h"

namespace DAVA
{
	// Different Property Names for Scene Nodes which are needed for Editor.
	static const char* SCENE_NODE_IS_SOLID_PROPERTY_NAME = Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME;
	static const char* SCENE_NODE_NAME_PROPERTY_NAME = "property.scenenode.name";
	static const char* SCENE_NODE_IS_VISIBLE_PROPERTY_NAME = "property.scenenode.isVisible";

	static const char* SCENE_NODE_USED_IN_STATIC_LIGHTING_PROPERTY_NAME = "Used in static lighting";
	static const char* SCENE_NODE_CAST_SHADOWS_PROPERTY_NAME = "Cast shadows";
	static const char* SCENE_NODE_RECEIVE_SHADOWS_PROPERTY_NAME = "Receive shadows";
};

#endif
