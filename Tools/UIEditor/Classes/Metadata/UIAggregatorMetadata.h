//
//  UIAggregatorControlMetadata.h
//  UIEditor
//
//  Created by adebt on 3/12/13.
//
//

#ifndef __UIEditor__UIAggregatorMetadata__
#define __UIEditor__UIAggregatorMetadata__

#include "UIControlMetadata.h"

namespace DAVA
{
	class UIAggregatorMetadata: public UIControlMetadata
	{
	public:
	    // Initialize the control(s) attached.
		virtual void InitializeControl(const String& controlName, const Vector2& position);
		
		virtual QString GetUIControlClassName();
	};
}

#endif /* defined(__UIEditor__UIAggregatorMetadata__) */
