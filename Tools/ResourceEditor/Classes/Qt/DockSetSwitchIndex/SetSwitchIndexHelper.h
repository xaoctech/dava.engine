#ifndef __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__
#define __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class Entity;
class SwitchComponent;

class SetSwitchIndexHelper
{
public:
	enum eSET_SWITCH_INDEX
	{
		FOR_SELECTED = 0,
		FOR_SCENE
	};

	static void ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state, Set<Entity*>& affectedEntities, Map<SwitchComponent *, int32>& originalIndexes);
	
	static void RestoreOriginalIndexes(Map<SwitchComponent *, int32>& originalIndexes, Set<Entity*>& affectedEntities);
};
};

#endif /* defined(__RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__) */
