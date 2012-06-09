#include "Debug/Backtrace.h"
#include "FileSystem/Logger.h"
#include <execinfo.h>


namespace DAVA 
{
//namespace Backtrace 
//{

    void PrintBackTraceToLog()
    {
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#define BACKTRACE_SIZ 100
        void    *array[BACKTRACE_SIZ];
        size_t  size, i;
        char    **strings;
        
        size = backtrace(array, BACKTRACE_SIZ);
        strings = backtrace_symbols(array, size);
        
        for (i = 0; i < size; ++i) {
            Logger::Debug("%p : %s\n", array[i], strings[i]);
        }
        
        free(strings);
#elif defined(__DAVAENGINE_WIN32__)
    // Check out this function http://msdn.microsoft.com/en-us/library/windows/desktop/bb204633(v=vs.85).aspx
    /* Should be able to do the same stuff on windows. 
         USHORT WINAPI CaptureStackBackTrace(
         __in       ULONG FramesToSkip,
         __in       ULONG FramesToCapture,
         __out      PVOID *BackTrace,
         __out_opt  PULONG BackTraceHash
         );        
     */
#endif
    }
//}

};