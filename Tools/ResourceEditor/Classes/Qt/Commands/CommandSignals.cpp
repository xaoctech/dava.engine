#include "CommandSignals.h"

void CommandSignals::EmitCommandDidAffectEntities(DAVA::Scene *scene,
												   CommandList::eCommandId id,
												   const DAVA::Set<DAVA::Entity *>& affectedEntities)
{
	emit CommandDidAffectEntities(scene, id, affectedEntities);
}