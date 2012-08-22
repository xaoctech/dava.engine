#include "Scene3D/ReferenceNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{

REGISTER_CLASS(ReferenceNode);

void ReferenceNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
	if(GetChildrenCount() == 1)
	{
		SceneNode * child = GetChild(0);
		String path = child->GetCustomProperties()->GetString("editor.referenceToOwner");
		String newPath = sceneFileV2->AbsoluteToRelative(path);
		customProperties->SetString("reference.path", newPath);
	}

	SceneNode::Save(archive, sceneFileV2);
}

void ReferenceNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
	SceneNode::Load(archive, sceneFileV2);

	if(customProperties->IsKeyExists("reference.path"))
	{
		const String & relativePath = customProperties->GetString("reference.path");
		String absolutePath = sceneFileV2->RelativeToAbsolute(relativePath);
		SceneNode * node = scene->GetRootNode(absolutePath);
		if(node)
		{
			AddNode(node);
		}
	}
}

};
