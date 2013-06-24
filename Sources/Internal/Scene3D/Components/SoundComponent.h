#ifndef __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__
#define __DAVAENGINE_SCENE3D_SOUND_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"

namespace DAVA 
{

class SoundEvent;
class SoundUpdateSystem;
class SoundComponent : public Component
{
public:
	SoundComponent();
	virtual ~SoundComponent();

	IMPLEMENT_COMPONENT_TYPE(SOUND_COMPONENT);

	SoundEvent * GetSoundEvent();
	const String & GetEventName();
	void SetEventName(const String & eventName);

	virtual Component * Clone(Entity * toEntity);
	virtual void Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile);
	virtual void Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile);

private:
	void SetSoundEvent(SoundEvent * sEvent);

	SoundEvent * soundEvent;
	String eventName;

	friend class SoundUpdateSystem;

public:
	INTROSPECTION_EXTEND(SoundComponent, Component,
		PROPERTY("eventName", "eventName", GetEventName, SetEventName, INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
		);
};

};

#endif