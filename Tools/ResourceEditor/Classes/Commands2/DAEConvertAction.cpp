/*==================================================================================
 Copyright (c) 2008, DAVA, INC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DAEConvertAction.h"
#include "Collada/ColladaConvert.h"

DAEConvertAction::DAEConvertAction(const DAVA::FilePath &path)
	: CommandAction(CMDID_DAE_CONVERT, "DAE to SC2 Convert")
	, daePath(path)
{ }

DAEConvertAction::~DAEConvertAction()
{ }

void DAEConvertAction::Redo()
{
	if(daePath.Exists() && daePath.IsEqualToExtension(".dae"))
	{
		eColladaErrorCodes code = ConvertDaeToSce(daePath.GetAbsolutePathname());
		if(code == COLLADA_OK)
		{
			// load sce to scene object
			DAVA::FilePath path(daePath);
			path.ReplaceExtension(".sce");

			DAVA::Scene *scene = new DAVA::Scene();
			DAVA::Entity *rootNode = scene->GetRootNode(path);
			scene->AddNode(rootNode);
			scene->BakeTransforms();

			// Export to *.sc2
			path.ReplaceExtension(".sc2");
			DAVA::SceneFileV2 * file = new DAVA::SceneFileV2();
			file->EnableDebugLog(false);
			file->SaveScene(path, scene);

			SafeRelease(file);
			SafeRelease(scene);
		}
		else if(code == COLLADA_ERROR_OF_ROOT_NODE)
		{
			DAVA::Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
		}
		else
		{
			DAVA::Logger::Error("Can't convert from DAE.");
		}
	}
}
