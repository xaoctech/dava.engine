#ifndef __DAVAENGINE_UI_AGGREGATOR_CONTROL_H__
#define __DAVAENGINE_UI_AGGREGATOR_CONTROL_H__

#include "UIControl.h"

namespace DAVA
{
	class UIAggregatorControl : public UIControl
	{
	public:
		UIAggregatorControl(const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
		virtual UIControl *Clone();

		virtual YamlNode* SaveToYamlNode(UIYamlLoader * loader);
		virtual void LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader);
		virtual List<UIControl* >& GetRealChildren();
	
		void AddAggregatorChild(UIControl* uiControl);
		
		void SetAggregatorPath(const String& path);
		String GetAggregatorPath() const;
			
	private:
		List<UIControl* > aggregatorControls;
		String aggregatorPath;
	};
};

#endif
