#include "Render/RenderOptions.h"

namespace DAVA
{

RenderOptions::RenderOptions()
{
	for(int32 i = 0; i < OPTIONS_COUNT; ++i)
	{
		options[i] = true;
	}
}

bool RenderOptions::IsOptionEnabled(RenderOption option)
{
	return options[option];
}

void RenderOptions::SetOption(RenderOption option, bool value)
{
	options[option] = value;
}

};