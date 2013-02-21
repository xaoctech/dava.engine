#ifndef __DAVAENGINE_COMMANDLINEPARSER_H__
#define __DAVAENGINE_COMMANDLINEPARSER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"


class CommandLineParser : public DAVA::StaticSingleton<CommandLineParser>
{
public:
	CommandLineParser();
	~CommandLineParser();
	
	void SetVerbose(bool isVerbose);
	bool GetVerbose();

	void SetExtendedOutput(bool isExO);
	bool IsExtendedOutput();
	
	void SetArguments(int argc, char * argv[]);
	void ClearFlags();
	void SetFlags(const DAVA::Vector<DAVA::String> & flags);
	bool	IsFlagSet(const DAVA::String & s);
	DAVA::uint32	GetFlagCount() { return (DAVA::uint32)flags.size(); };
	DAVA::String	GetParam(int index);
	DAVA::uint32	GetParamCount() { return (DAVA::uint32)params.size(); };
	

	static void		SplitFilePath(const DAVA::String & filepath, DAVA::String & path, DAVA::String & filename);
	static void		RemoveFromPath(DAVA::String & path, const DAVA::String & removePart);	
	static DAVA::String	ReplaceExtension(const DAVA::String & filename, const DAVA::String & nexExt);
	static DAVA::String	GetExtension(const DAVA::String & filename);
	
	static DAVA::String RealPath(DAVA::String path);

private:
	DAVA::Vector<DAVA::String>	params;
	DAVA::Vector<DAVA::String>	flags;
	bool isVerbose;
	bool isExtendedOutput;
};


#endif // __DAVAENGINE_COMMANDLINEPARSER_H__