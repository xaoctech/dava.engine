/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/



#ifndef __RESOURCEEDITORQT__ADDSKYBOXDIALOG__
#define __RESOURCEEDITORQT__ADDSKYBOXDIALOG__

#include "../BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "DAVAEngine.h"
#include "Qt/Scene/SceneEditor2.h"
#include <QPushButton>

class AddSkyboxDialog : public BaseAddEntityDialog
{
	Q_OBJECT
	
public:
	
	AddSkyboxDialog(QWidget* parent = 0);
	
	~AddSkyboxDialog();
	
	static void Show(QWidget* parent, SceneEditor2* scene);
	
	void SetInitialState(Entity* skyboxState);
	void SetEditorScene(SceneEditor2* scene);
	
	SceneEditor2* GetEditorScene() const;
	
protected slots:
	
	void OnFinished(int code);
	void OnCreateButtonClicked();
	void OnDeleteButtonClicked();
	
private:
	
	void MakeCreateButton();
	void MakeDeleteButton();
	
	void UpdateEntity(Entity* newEntity);
	
private:
	
	struct InitialSkyboxState
	{
		Entity* initialSkyboxNode;
		DAVA::float32 offset;
		DAVA::float32 rotation;
		FilePath texture;
	};
	
private:
	
	QPushButton* controlButton;
	bool closeHandled;
	SceneEditor2* editorScene;

	InitialSkyboxState initialState;
	
};

#endif /* defined(__RESOURCEEDITORQT__ADDSWITCHENTITYDIALOG__) */