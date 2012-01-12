#ifndef __EDITOR_LIGHT_NODE_H__
#define __EDITOR_LIGHT_NODE_H__

#include "DAVAEngine.h"
using namespace DAVA;

class EditorLightNode : public SceneNode
{
public:
	EditorLightNode(Scene * _scene);
	virtual ~EditorLightNode();

	static LightNode * CreateSceneAndEditorLight(Scene * scene);

private:
};

#endif //__EDITOR_LIGHT_NODE_H__
