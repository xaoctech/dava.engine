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

#ifndef __RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__
#define __RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__

#include "DAVAEngine.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData.h"
#include "Tools/QtPropertyEditor/QtProperyData/QtPropertyDataMetaObject.h"

class GeneralSettingsEditor: public QtPropertyEditor
{
	Q_OBJECT
	
public:
	explicit GeneralSettingsEditor(QWidget* parent = 0);
	
	~GeneralSettingsEditor();

protected slots:
	
void	OnValueChanged();
	
protected:
	
	void InitializeProperties();
	
	QtPropertyItem* AddHeader(const char *text);
	QtPropertyDataMetaObject* AddPropertyItem(const char *name, DAVA::BaseObject *object, QtPropertyItem *parent);
	
	QtPropertyDataDavaVariant *propertyScreenWidth;
	QtPropertyDataDavaVariant *propertyScreenHeight;
	QtPropertyDataDavaVariant *propertyLanguage;
	QtPropertyDataDavaVariant *propertyOutput;
	QtPropertyDataDavaVariant *propertyCameraSpeed1;
	QtPropertyDataDavaVariant *propertyCameraSpeed2;
	QtPropertyDataDavaVariant *propertyCameraSpeed3;
	QtPropertyDataDavaVariant *propertyCameraSpeed4;
	QtPropertyDataDavaVariant *propertyLeftpaneWidth;
	QtPropertyDataDavaVariant *propertyRightpaneWidth;
	QtPropertyDataDavaVariant *propertyDrawGrid;
	QtPropertyDataDavaVariant *propertyImposters;
	QtPropertyDataDavaVariant *propertyDesignerName;
	QtPropertyDataDavaVariant *propertyPreviewAtLibrary;
	
	DAVA::Vector<QtPropertyDataDavaVariant *> propertiesSet;
	
};
#endif /* defined(__RESOURCEEDITORQT__GENERAL_SETTINGS_EDITOR__) */