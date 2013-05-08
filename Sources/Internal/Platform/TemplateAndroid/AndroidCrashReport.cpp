#include "AndroidCrashReport.h"

#include "FileSystem/Logger.h"
#include "JniExtensions.h"
#include "FileSystem/File.h"

#include <dlfcn.h>
#include <unistd.h>


using namespace DAVA;

typedef uint32_t  kernel_sigmask_t[2];
typedef struct ucontext {
	uint32_t uc_flags;
	struct ucontext* uc_link;
	stack_t uc_stack;
	sigcontext uc_mcontext;
	kernel_sigmask_t uc_sigmask;
} ucontext_t;

static int fatalSignals[] = {
	SIGABRT,
	SIGBUS,
	SIGFPE,
	SIGILL,
	SIGSEGV,
	SIGTRAP
};

static int fatalSignalsCount = (sizeof(fatalSignals) / sizeof(fatalSignals[0]));

stack_t AndroidCrashReport::s_sigstk;
CustomReport* AndroidCrashReport::s_customReport = NULL;

class JniCrashReporter: public JniExtension
{
public:
	JniCrashReporter();
	String GetReportFile();
};

JniCrashReporter::JniCrashReporter()
:	JniExtension("com/dava/framework/JNICrashReporter")
{
}

String JniCrashReporter::GetReportFile()
{
	jclass javaClass = GetJavaClass();
	if (!javaClass)
		return "";

	char path[255];
	jmethodID mid = GetMethodID(javaClass, "GetReportFile", "()Ljava/lang/String;");
	//jmethodID mid = GetMethodID(javaClass, "GetReportFile", "()V");
	if (mid)
	{
		jobject obj = env->CallStaticObjectMethod(javaClass, mid, 0);
		jstring jStr = (jstring)obj;
		char const* nativeString = env->GetStringUTFChars(jStr, 0);
		strncpy(path, nativeString, 255);
		env->ReleaseStringUTFChars(jStr, nativeString);
	}

	ReleaseJavaClass(javaClass);

	return String(path);
}

String GetInfo(void* pc)
{
	char buff[255];

	Dl_info dlInfo;
	if (dladdr(pc, &dlInfo))
		sprintf(buff, "pc=0x%x, module=%s, function=%s", pc, dlInfo.dli_fname, dlInfo.dli_sname);
	else
		sprintf(buff, "pc=0x%x", pc);

	return String(buff);
}

void AndroidCrashReport::WriteCStack(File* file, void *uapVoid)
{
	file->WriteLine("C++CallStack");
	ucontext_t *crashctx = (ucontext_t*) uapVoid;

	file->WriteLine(GetInfo((void*)crashctx->uc_mcontext.arm_pc));
	//file->WriteLine(GetInfo((void*)crashctx->uc_mcontext.arm_lr));

	file->WriteLine("\n");
}

void AndroidCrashReport::SignalHandler(int signal, siginfo_t *info, void *uapVoid)
{
	for (int i = 0; i < fatalSignalsCount; i++)
	{
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = SIG_DFL;
		sigemptyset(&sa.sa_mask);
		sigaction(fatalSignals[i], &sa, NULL);
	}

	JniCrashReporter reporter;
	String path = reporter.GetReportFile();

	File* file = File::Create(path, File::WRITE | File::APPEND);
	if (file)
	{
		WriteCStack(file, uapVoid);

		if (s_customReport)
			s_customReport->WriteCustomReport(file, signal, info, uapVoid);
	}
	SafeRelease(file);


	//void** fp = (void**) crashctx->uc_mcontext.arm_fp;
	//void* pc = *((void**)*fp);

	//GetInfo((void*)pc);

    /*while (true)
    {
    	fp++;
    	if (pc == (void*) *fp)
    	{
    		fp--;
    		pc = *((void**)*((void**)fp));
    		fp++;
    		if (!GetInfo(pc))
    			break;
    	}
    }*/

	//LOGE("END STACK");

	kill(getpid(), SIGKILL);
}

void AndroidCrashReport::Init()
{
	s_sigstk.ss_size = 64 * 1024;
	s_sigstk.ss_sp = malloc(s_sigstk.ss_size);
	s_sigstk.ss_flags = 0;
	
	if (sigaltstack(&s_sigstk, 0) < 0)
	{
		Logger::Error("Could not initialize alternative signal stack");
	}
	
	for (int i = 0; i < fatalSignalsCount; i++)
	{
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_flags = SA_SIGINFO|SA_ONSTACK;
		sigemptyset(&sa.sa_mask);
		sa.sa_sigaction = &SignalHandler;
		
		if (sigaction(fatalSignals[i], &sa, NULL) != 0)
		{
			Logger::Error("Signal registration for failed:");
		}
	}
}

void AndroidCrashReport::SetCustomReport(CustomReport* logger)
{
	s_customReport = logger;
}
