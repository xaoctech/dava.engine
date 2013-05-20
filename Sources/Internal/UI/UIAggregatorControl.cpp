//
//  UIAggregatorControl.cpp
//  Framework
//
//  Created by adebt on 3/12/13.
//
//

#include "UIAggregatorControl.h"
#include "FileSystem/FileSystem.h"

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
	node->Set(AGGREGATOR_PATH, aggregatorPath.GetAbsolutePathname());
	return node;
}

void UIAggregatorControl::LoadFromYamlNode(YamlNode * node, UIYamlLoader * loader)
{
	UIControl::LoadFromYamlNode(node, loader);
	
	YamlNode * pathNode = node->Get(AGGREGATOR_PATH);
	if (pathNode)
	{
		aggregatorPath = FilePath(pathNode->AsString());
		String aggregatorFileName = aggregatorPath.GetFilename();

		aggregatorPath = loader->GetCurrentPath() + aggregatorFileName;

		UIYamlLoader loader;
		loader.Load(this, aggregatorPath);

		aggregatorPath = FilePath(aggregatorFileName);
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

void UIAggregatorControl::AddAggregatorChild(UIControl* uiControl)
{
	//AddControl(uiControl);
	//BringChildFront(uiControl);
	aggregatorControls.push_back(uiControl);
}

void UIAggregatorControl::SetAggregatorPath(const FilePath& path)
{
	aggregatorPath = path;
}

const FilePath & UIAggregatorControl::GetAggregatorPath() const
{
	return aggregatorPath;
}