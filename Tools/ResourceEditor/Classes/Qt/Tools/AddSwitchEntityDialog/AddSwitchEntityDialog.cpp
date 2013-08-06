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

#include "AddSwitchEntityDialog.h"
#include "./../Qt/Tools/MimeDataHelper/MimeDataHelper.h"
#include "./../Qt/Tools/SelectPathWidget/SelectPathWidget.h"


AddSwitchEntityDialog::AddSwitchEntityDialog(DAVA::Entity* entityToDisplay, QWidget* parent)
		:BaseAddEntityDialog(entityToDisplay,parent)
{
	setAcceptDrops(false);
	
		
	SelectPathWidget* firstWidget = new SelectPathWidget(parent);
	SelectPathWidget* secondWidget = new SelectPathWidget(parent);
	SelectPathWidget* thirdWidget = new SelectPathWidget(parent);

	firstWidget->SetDiscriptionText("First Entity:");
	secondWidget->SetDiscriptionText("Second Entity:");
	thirdWidget->SetDiscriptionText("Third Entity:");
	
	AddControlToUserContainer(firstWidget);
	AddControlToUserContainer(secondWidget);
	AddControlToUserContainer(thirdWidget);

	pathWidgets.push_back(firstWidget);
	pathWidgets.push_back(secondWidget);
	pathWidgets.push_back(thirdWidget);
}

AddSwitchEntityDialog::~AddSwitchEntityDialog()
{
	RemoveAllControlsFromUserContainer();
	
	Q_FOREACH(SelectPathWidget* widget, pathWidgets)
	{
		delete widget;
	}
}

void AddSwitchEntityDialog::CleanupPathWidgets()
{
	Q_FOREACH(SelectPathWidget* widget, pathWidgets)
	{
		widget->EraseWidget();
	}
}

void AddSwitchEntityDialog::GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, SceneEditor2* editor)
{
	Q_FOREACH(SelectPathWidget* widget, pathWidgets)
	{
		entities.push_back(widget->GetOutputEntity(editor));
	}
}
