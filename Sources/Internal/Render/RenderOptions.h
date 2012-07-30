#ifndef __DAVAENGINE_RENDEROPTIONS_H__
#define __DAVAENGINE_RENDEROPTIONS_H__

#include "Base/BaseTypes.h"

namespace DAVA
{

class RenderOptions
{
public:
	enum RenderOption
	{
		LANDSCAPE_DRAW = 0,
		
		OPAQUE_DRAW,

		IMPOSTERS_ENABLE,

		MESHINSTANCE_DRAW,
		MESHINSTANCE_UPDATEDRAW,

		OPTIONS_COUNT
	};

	bool IsOptionEnabled(RenderOption option);
	void SetOption(RenderOption option, bool value);

private:
	RenderOptions();

	bool options[OPTIONS_COUNT];

	friend class RenderManager;
};

};

#endif //__DAVAENGINE_RENDEROPTIONS_H__
