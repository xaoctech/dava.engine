#ifndef __DAVAENGINE_SWITCH_COMPONENT_H__
#define __DAVAENGINE_SWITCH_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Debug/DVAssert.h"
#include "Base/Introspection.h"

namespace DAVA
{

class SwitchComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(SWITCH_COMPONENT);

	SwitchComponent();
	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

	void SetSwitchIndex(const int32 & switchIndex);
	int32 GetSwitchIndex() const;

private:
	int32 oldSwitchIndex;
	int32 newSwitchIndex;

	friend class SwitchSystem;

public:
	INTROSPECTION_EXTEND(SwitchComponent, Component,
		PROPERTY("newSwitchIndex", "Switch index", GetSwitchIndex, SetSwitchIndex, INTROSPECTION_EDITOR)
		);
};

}
#endif //__DAVAENGINE_SWITCH_COMPONENT_H__
