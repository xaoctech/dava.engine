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



#ifndef __RESOURCEEDITORQT__LANDSCAPEDIALOG__
#define __RESOURCEEDITORQT__LANDSCAPEDIALOG__

#include <QVBoxLayout>
#include <QPushButton.h>
#include <QDialogButtonBox>

#include "DAVAEngine.h"
#include "Qt/Scene/SceneEditor2.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "./../Qt/Tools/SelectPathWidget/SelectPathWidgetBase.h"
#include "Tools/BaseAddEntityDialog/BaseAddEntityDialog.h"
#include "LandscapeSettingsEditor.h"

class SelectEntityPathWidget;

class LandscapeDialog: public BaseAddEntityDialog
{
	Q_OBJECT
	
public:
	LandscapeDialog(Entity* landscapeEntity, QWidget* parent = 0);
	
	~LandscapeDialog();
	
	void CleanupPathWidgets();

	Entity* GetEntity()
	{
		return innerLandscapeEntity;
	}
	
public slots:
	
	void reject();
	
protected slots:
	
	void ValueChanged(DAVA::String fileName);
	
	void TileModeChanged(int newValue);
	
	void ActionButtonClicked();
	
protected:
		
	void SetLandscapeEntity(Entity* _landscapeEntity);

	SelectPathWidgetBase* FindWidgetBySpecInfo(int value);
	
	Landscape*				innerLandscape;
	Entity*					innerLandscapeEntity;
	Entity*					defaultLandscapeEntity;
	
	QPushButton*			actionButton;
	
	struct DefaultInfo
	{
		int32 specificInfo;
		FilePath path;
		DefaultInfo()
		{
			specificInfo = 0;
		}
		
		DefaultInfo(int32 _specificInfo, FilePath _path)
		{
			specificInfo = _specificInfo;
			path  = _path;
		}
	};
	
	DAVA::Map<SelectPathWidgetBase*,DefaultInfo>  widgetMap;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEDIALOG__) */