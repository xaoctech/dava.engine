#ifndef __DAVAENGINE_COMMANDLINEPARSER_H__
#define __DAVAENGINE_COMMANDLINEPARSER_H__

#include "Base/BaseTypes.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{

class CommandLineParser : public StaticSingleton<CommandLineParser>
{
public:
	CommandLineParser();
	virtual ~CommandLineParser();
	
	void SetVerbose(bool isVerbose);
	bool GetVerbose();

	void SetExtendedOutput(bool isExO);
	bool IsExtendedOutput();
	
	void SetArguments(int argc, char * argv[]);
	void ClearFlags();
	void SetFlags(const Vector<String> & flags);
	bool	IsFlagSet(const String & s);
    
	uint32	GetFlagCount() { return (uint32)flags.size(); };
	String	GetParam(int index);
	uint32	GetParamCount() { return (uint32)params.size(); };
	
	static void RemoveFromPath(String & path, const String & removePart);
	static String RealPath(String path);

private:
    
	Vector<String>	params;
	Vector<String>	flags;
	bool isVerbose;
	bool isExtendedOutput;
};

};

    
#endif // __DAVAENGINE_COMMANDLINEPARSER_H__