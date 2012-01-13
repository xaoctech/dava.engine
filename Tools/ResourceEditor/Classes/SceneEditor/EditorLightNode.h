#ifndef __EDITOR_LIGHT_NODE_H__
#define __EDITOR_LIGHT_NODE_H__

#include "DAVAEngine.h"
using namespace DAVA;

class EditorLightNode : public SceneNode
{
public:
	EditorLightNode(Scene * _scene = 0);
	virtual ~EditorLightNode();

	virtual void Update(float32 timeElapsed);
	virtual void Draw();

	static LightNode * CreateSceneAndEditorLight(Scene * scene);

private:

	LightNode::eType type;

	String GetSceneFile();
};

#endif //__EDITOR_LIGHT_NODE_H__
