#ifndef __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__
#define __RESOURCE_EDITOR_VISIBILITY_CHECK_TOOL_COMMANDS_H__

#include "Command.h"
#include "DAVAEngine.h"
#include "../SceneEditor/LandscapeEditorVisibilityCheckTool.h"

using namespace DAVA;

class CommandToggleVisibilityTool: public Command
{
public:
	CommandToggleVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSetAreaVisibilityTool: public Command
{
public:
	CommandSetAreaVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSetPointVisibilityTool: public Command
{
public:
	CommandSetPointVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandSaveTextureVisibilityTool: public Command
{
public:
	CommandSaveTextureVisibilityTool();
	
protected:
	virtual void Execute();
};

class CommandChangeAreaSizeVisibilityTool: public Command
{
public:
	CommandChangeAreaSizeVisibilityTool(uint32 newSize);
	
protected:
	uint32 size;
	
	virtual void Execute();
};

class CommandPlacePointVisibilityTool: public Command
{
public:
	CommandPlacePointVisibilityTool(const Vector2& newVisibilityPoint, const Vector2& oldVisibilityPoint, bool oldPointIsSet, Image* oldImage);
	virtual ~CommandPlacePointVisibilityTool();

protected:
	Vector2 point;
	LandscapeEditorVisibilityCheckTool* editor;

	Vector2 oldPoint;
	bool oldPointIsSet;
	Image* oldImage;

	LandscapeEditorVisibilityCheckTool* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};

class CommandPlaceAreaVisibilityTool: public Command
{
public:
	CommandPlaceAreaVisibilityTool(const Vector2& areaPoint, uint32 areaSize, Image* oldImage);
	virtual ~CommandPlaceAreaVisibilityTool();

protected:
	Vector2 point;
	uint32 size;

	Image* oldImage;
	Image* redoImage;

	LandscapeEditorVisibilityCheckTool* GetEditor();

	virtual void Execute();
	virtual void Cancel();
};

#endif