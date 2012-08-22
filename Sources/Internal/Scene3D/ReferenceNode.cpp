#include "Scene3D/ReferenceNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "FileSystem/KeyedArchive.h"

namespace DAVA
{

REGISTER_CLASS(ReferenceNode);

ReferenceNode::ReferenceNode()
{
	nodeToAdd = 0;
}

void ReferenceNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
	String savedPath = "";
	if(customProperties && customProperties->IsKeyExists("reference.path"))
	{
		savedPath = customProperties->GetString("reference.path");
		String newPath = sceneFileV2->AbsoluteToRelative(savedPath);
		customProperties->SetString("reference.path", newPath);
	}

	SceneNode::Save(archive, sceneFileV2);

	if(customProperties && savedPath.length())
	{
		customProperties->SetString("reference.path", savedPath);
	}
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
			nodeToAdd = node->Clone();
		}
	}
}

void ReferenceNode::GetDataNodes(Set<DataNode*> & dataNodes)
{
}

void ReferenceNode::Update(float32 timeElapsed)
{
	if(nodeToAdd)
	{
		AddNode(nodeToAdd);
		SafeRelease(nodeToAdd);
	}

	SceneNode::Update(timeElapsed);
}

SceneNode* ReferenceNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		dstNode = new ReferenceNode();
	}

	return SceneNode::Clone(dstNode);
}

};
