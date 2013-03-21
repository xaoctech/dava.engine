#ifndef __DAVAENGINE_DVASSERT_MESSAGE_H__
#define __DAVAENGINE_DVASSERT_MESSAGE_H__

#include "DAVAConfig.h"
#include "Base/BaseTypes.h"
namespace DAVA 
{
	class DVAssertMessage
	{
	public:

		static void ShowMessage(const char8 * text, ...);

	protected:

		static void InnerShow(const char* content);
	};
};
#endif // __DAVAENGINE_DVASSERT_MESSAGE_H__