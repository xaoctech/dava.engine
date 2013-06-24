#ifndef __DAVAENGINE_ANDROID_CRASH_REPORT_H__
#define __DAVAENGINE_ANDROID_CRASH_REPORT_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <signal.h>

namespace DAVA
{

class File;

class CustomReport
{
public:
	virtual void WriteCustomReport(File*, int signal, siginfo_t *info, void *uapVoid);
};

class AndroidCrashReport
{
public:
	static void Init();
	static void SetCustomReport(CustomReport* );

private:
	static void SignalHandler(int signal, siginfo_t *info, void *uapVoid);
	static void WriteCStack(File*, void *uapVoid);

private:
	static stack_t s_sigstk;
	static CustomReport* s_customReport;
};

}

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif /* #ifndef __DAVAENGINE_ANDROID_CRASH_HANDLER_H__ */
