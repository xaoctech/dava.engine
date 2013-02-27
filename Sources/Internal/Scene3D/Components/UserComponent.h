#ifndef __DAVAENGINE_USER_COMPONENT_H__
#define __DAVAENGINE_USER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"

namespace DAVA
{
	class UserComponent : public Component
	{
	public:
		IMPLEMENT_COMPONENT_TYPE(USER_COMPONENT);

		UserComponent();
		virtual Component * Clone(SceneNode * toEntity);
		virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
		virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

	public:
		/*
		INTROSPECTION_EXTEND(UserComponent, Component,
			NULL
			);
		*/
	};
}
#endif //__DAVAENGINE_USER_COMPONENT_H__
