#include "EditorLightNode.h"

EditorLightNode::EditorLightNode(Scene * _scene)
: SceneNode(scene)
{

}

EditorLightNode::~EditorLightNode()
{

}

LightNode * EditorLightNode::CreateSceneAndEditorLight(Scene * scene)
{
	LightNode * ret = new LightNode(scene);
	EditorLightNode * child = new EditorLightNode(scene);
	child->SetName("editor.light");
	ret->AddNode(child);

	SafeRelease(child);
	return ret;
}
