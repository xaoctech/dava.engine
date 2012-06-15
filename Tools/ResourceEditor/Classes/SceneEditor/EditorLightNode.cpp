#include "EditorLightNode.h"


REGISTER_CLASS(EditorLightNode);

EditorLightNode::EditorLightNode()
:	SceneNode(),
	type(LightNode::TYPE_COUNT)
{

}

EditorLightNode::~EditorLightNode()
{

}

LightNode * EditorLightNode::CreateSceneAndEditorLight() 
{
	LightNode * ret = new LightNode();
	ret->SetSolid(true);
	EditorLightNode * child = new EditorLightNode();
	child->SetName("editor.light");
	ret->AddNode(child);

	SafeRelease(child);
	return ret;
}

void EditorLightNode::Update(float32 timeElapsed)
{
	SceneNode::Update(timeElapsed);
	LightNode * parent = (LightNode*)GetParent();
	if(type != parent->GetType())
	{
		RemoveAllChildren();

		type = parent->GetType();
		SceneNode * lightDrawNode = scene->GetRootNode(GetSceneFile())->Clone();
		AddNode(lightDrawNode);
		SafeRelease(lightDrawNode);
	}
}

void EditorLightNode::Draw()
{
	SceneNode::Draw();
}

DAVA::String EditorLightNode::GetSceneFile()
{
	switch(type)
	{
	case LightNode::TYPE_SKY:
		return "~res:/3d/lights/skylight/skylight.sc2";
		break;
	case LightNode::TYPE_DIRECTIONAL:
		return "~res:/3d/lights/directlight/directlight.sc2";
		break;
	case LightNode::TYPE_POINT:
		return "~res:/3d/lights/pointlight/pointlight.sc2";
		break;
	default:
		return String();
	}
}

SceneNode* EditorLightNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if(!dstNode)
	{
		dstNode = new EditorLightNode();
	}

	SceneNode::Clone(dstNode);

	EditorLightNode *lightNode = (EditorLightNode *)dstNode;
	lightNode->type = type;

	return dstNode;
}
