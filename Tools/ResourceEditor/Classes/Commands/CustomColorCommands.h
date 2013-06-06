#ifndef __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__
#define __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../Constants.h"
#include "../SceneEditor/LandscapeEditorCustomColors.h"

using namespace DAVA;

class CommandSaveTextureCustomColors: public Command
{
public:
    DAVA_DEPRECATED(CommandSaveTextureCustomColors());// DEPRECATED: using QFileDialog
protected:
    virtual void Execute();
};

class CommandLoadTextureCustomColors: public Command
{
public:
	DAVA_DEPRECATED(CommandLoadTextureCustomColors());// DEPRECATED : using QFileDialog
protected:
    virtual void Execute();
};

class CommandDrawCustomColors: public Command
{
public:
	DAVA_DEPRECATED(CommandDrawCustomColors(Image* originalImage, Image* newImage));// DEPRECATED: using SceneDataManager
	virtual ~CommandDrawCustomColors();
protected:
	Image* undoImage;
	Image* redoImage;

	LandscapeEditorCustomColors* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};


class CustomColorsProxy;

class CommandModifyCustomColors: public Command
{
public:
	CommandModifyCustomColors(Image* originalImage,
							  CustomColorsProxy* customColorsProxy,
							  const Rect& updatedRect);
	virtual ~CommandModifyCustomColors();

protected:
	CustomColorsProxy* customColorsProxy;
	Image* undoImage;
	Image* redoImage;
	Rect updatedRect;

	virtual void Execute();
	virtual void Cancel();
};

#endif // #ifndef __RESOURCE_EDITOR_CUSTOM_COLOR_COMMANDS_H__