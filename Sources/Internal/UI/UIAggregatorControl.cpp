//
//  UIAggregatorControl.cpp
//  Framework
//
//  Created by adebt on 3/12/13.
//
//

#include "UIAggregatorControl.h"

using namespace DAVA;

#define AGGREGATOR_PATH "aggregatorPath"

REGISTER_CLASS(UIAggregatorControl);

UIAggregatorControl::UIAggregatorControl(const Rect &rect, bool rectInAbsoluteCoordinates) :
	UIControl(rect, rectInAbsoluteCoordinates)
{
	
}


UIControl* UIAggregatorControl::Clone()
{
	UIAggregatorControl* c = new UIAggregatorControl(Rect(relativePosition.x, relativePosition.y, size.x, size.y));
	c->CopyDataFrom(this);
	return c;
}

YamlNode* UIAggregatorControl::SaveToYamlNode(UIYamlLoader * loader)
{
	YamlNode* node = UIControl::SaveToYamlNode(loader);
	SetPreferredNodeType(node, "UIAggregatorControl");
	node->Set(AGGREGATOR_PATH, aggregatorPath);
	return node;
}

void UIAggregatorControl::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	YamlNode * pathNode = node->Get(AGGREGATOR_PATH);
	if (pathNode)
	{
		aggregatorPath = pathNode->AsString();
		UIYamlLoader loader;
		loader.Load(this, aggregatorPath);
	}
}

List<UIControl* >& UIAggregatorControl::GetRealChildren()
{
	realChilds = UIControl::GetRealChildren();
	
	for (List<UIControl*>::iterator iter = aggregatorControls.begin(); iter != aggregatorControls.end(); ++iter)
	{
		UIControl* control = (*iter);
		realChilds.remove(control);
	}
	return realChilds;
}

void UIAggregatorControl::CleanAggregatorChilds()
{
	
}

void UIAggregatorControl::AddAggregatorChild(UIControl* uiControl)
{
	//AddControl(uiControl);
	//BringChildFront(uiControl);
	aggregatorControls.push_back(uiControl);
}

void UIAggregatorControl::SetAggregatorPath(const String& path)
{
	aggregatorPath = path;
}

String UIAggregatorControl::GetAggregatorPath() const
{
	return aggregatorPath;
}