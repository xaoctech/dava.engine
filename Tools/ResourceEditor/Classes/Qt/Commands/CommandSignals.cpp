#include "CommandSignals.h"

void CommandSignals::EmitCommandAffectsEntities(DAVA::Scene *scene,
												   CommandList::eCommandId id,
												   const DAVA::Set<DAVA::Entity *>& affectedEntities)
{
	emit CommandAffectsEntities(scene, id, affectedEntities);
}