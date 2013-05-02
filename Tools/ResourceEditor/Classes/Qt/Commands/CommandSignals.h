#ifndef __RESOURCEEDITORQT__COMMANDSIGNALS__
#define __RESOURCEEDITORQT__COMMANDSIGNALS__

#include <QObject>

#include "Base/StaticSingleton.h"
#include "Scene3D/Entity.h"

#include "Commands/CommandList.h"

class CommandSignals: public QObject, public DAVA::StaticSingleton<CommandSignals>
{
	Q_OBJECT

signals:
	void CommandAffectsEntities(DAVA::Scene* scene,
								  CommandList::eCommandId id,
								  const DAVA::Set<DAVA::Entity*>& affectedEntities);

public:
	void EmitCommandAffectsEntities(DAVA::Scene* scene,
									  CommandList::eCommandId id,
									  const DAVA::Set<DAVA::Entity*>& affectedEntities);
};

#endif /* defined(__RESOURCEEDITORQT__COMMANDSIGNALS__) */
