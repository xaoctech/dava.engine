#ifndef __DAVAENGINE_DVASSERT_MESSAGE_H__
#define __DAVAENGINE_DVASSERT_MESSAGE_H__

#include "DAVAConfig.h"
#include "Base/BaseTypes.h"

namespace DAVA {

class DVAssertMessage
{
public:
	
	// Modality type,
	enum eModalType
	{
		// Try to show non-modal assert message on the platforms where it is applicable.
		TRY_NONMODAL,

		// Always show the assertion message as modal.
		ALWAYS_MODAL
	};

	static void ShowMessage(eModalType modalType, const char8 * text, ...);

protected:
	static void InnerShow(eModalType modalType, const char* content);
	
	// Pointer to the platform-specific message.
	static void* messageBoxPtr;
};

};
#endif // __DAVAENGINE_DVASSERT_MESSAGE_H__