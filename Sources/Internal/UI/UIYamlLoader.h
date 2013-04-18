/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Alexey 'Hottych' Prosin
=====================================================================================*/

#ifndef __DAVAENGINE_YAML_LOADER_H__
#define __DAVAENGINE_YAML_LOADER_H__

#include "Base/BaseTypes.h"
#include "Render/2D/FTFont.h"


namespace DAVA 
{
/**
	\ingroup controlsystem
	\brief Class to perform loading of controls from yaml configs
	
	This class can help you to load hierarhy of controls from yaml config file.
	Structure of yaml file is very simple and it allow you to modify the structure of the application easily using 
	configuration files.
*/ 

class UIControl;
class YamlNode;
class UIYamlLoader : public BaseObject
{
public:
	UIYamlLoader();

	/**
		\brief	This is main function in UIYamlLoader and it loads control hierarchy from yamlPathname file and add it to 
				rootControl.
		
		\param[in, out]	rootControl					we add all created control classes to this control
		\param[in] yamlPathName						we get config file using this pathname
 		\param[in] assertIfCustomControlNotFound	if this flag is set to true, ASSERT and stop app execution if the
													custom control can't be loaded.
	 */
	static void Load(UIControl * rootControl, const String & yamlPathname,
					 bool assertIfCustomControlNotFound = false);

	//Internal functions that do actual loading and saving.
	void ProcessLoad(UIControl * rootControl, const String & yamlPathname);
	void LoadFromNode(UIControl * rootControl, YamlNode * node, bool needParentCallback);
	
	/**
     \brief	This function saves the UIControl's hierarchy to the YAML file passed.
     rootControl.
     
     \param[in, out]	rootControl		is used to take the configuration from
     \param[in]			yamlPathName	path to store hierarchy too
     \return            true if the save was successful
	 */
	static bool Save(UIControl * rootControl, const String & yamlPathname, bool skipRootNode);
	
    YamlNode* SaveToNode(UIControl * parentControl, YamlNode * rootNode, int relativeDepth = 0);
	bool ProcessSave(UIControl * rootControl, const String & yamlPathname, bool skipRootNode);

	Font * GetFontByName(const String & fontName);
	
    int32 GetDrawTypeFromNode(YamlNode * drawTypeNode);
	int32 GetColorInheritTypeFromNode(YamlNode * colorInheritNode);
	int32 GetAlignFromYamlNode(YamlNode * align);
	Rect GetRectFromYamlNode(YamlNode * rect);
	bool GetBoolFromYamlNode(YamlNode * node, bool defaultValue);
	Color GetColorFromYamlNode(YamlNode * node);
	
    String GetColorInheritTypeNodeValue(int32 colorInheritType);
    String GetDrawTypeNodeValue(int32 drawType);
	YamlNode * GetAlignNodeValue(int32 align);
	
	Map<String, Font*> fontMap;

	// Set the "ASSERT if custom control is not found during loading" flag.
	void SetAssertIfCustomControlNotFound(bool value);

	String GetCurrentPath() const;

protected:
	// Create the control by its type or base type.
	UIControl* CreateControl(const String& type, const String& baseType);

	// ASSERTion flag for "Custom Control not found" state.
	bool assertIfCustomControlNotFound;

	String currentPath;
};
};

#endif // __DAVAENGINE_YAML_LOADER_H__