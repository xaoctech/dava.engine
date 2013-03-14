#ifndef __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__
#define __RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
class SetSwitchIndexHelper
{
public:
	enum eSET_SWITCH_INDEX
	{
		FOR_SELECTED = 0,
		FOR_SCENE
	};

	static void ProcessSwitchIndexUpdate(uint32 value, eSET_SWITCH_INDEX state);
};
};

#endif /* defined(__RESOURCEEDITORQT__SET_SWITCH_INDEX_HELPER_H__) */
