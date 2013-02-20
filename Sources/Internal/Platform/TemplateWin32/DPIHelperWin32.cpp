#include "Platform/DPIHelper.h"


namespace DAVA
{
	uint32 DPIHelper::GetScreenDPI()
	{
		HDC screen = GetDC(NULL);

		// in common dpi is the same in horizontal and vertical demensions
		// in any case under win this value is 96dpi due to OS limitation
		uint32 hDPI = GetDeviceCaps(screen, LOGPIXELSX);
		ReleaseDC(NULL, screen);
		return hDPI;
	}
}