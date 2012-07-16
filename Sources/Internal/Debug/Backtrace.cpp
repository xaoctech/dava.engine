#include "Debug/Backtrace.h"
#include "FileSystem/Logger.h"
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#include <execinfo.h>
#include <cxxabi.h>
#endif


namespace DAVA 
{
//namespace Backtrace 
//{
    Backtrace * CreateBacktrace()
    {
        Backtrace * bt = (Backtrace*) malloc(sizeof(Backtrace));
        //GetBacktrace(bt);
        return bt;
    }
    
    void GetBacktrace(Backtrace * bt)
    {
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
        bt->size = backtrace(bt->array, MAX_BACKTRACE_DEPTH);
#elif defined(__DAVAENGINE_WIN32__)
        bt->size = CaptureStackBackTrace( 0, MAX_BACKTRACE_DEPTH, bt->array, NULL);
#endif
        pointer_size hash = 0;
        for (uint32 k = 0; k < bt->size; ++k)
        {
            hash += (k + 1) * (pointer_size)bt->array[k];
        }
        bt->hash = hash;
    }
    
    void ReleaseBacktrace(Backtrace * backtrace)
    {
        free(backtrace);
    }
    
    void CreateBacktraceLog(Backtrace * backtrace, BacktraceLog * log)
    {
        log->strings = (char**)malloc(sizeof(char*) * backtrace->size); 
        log->size = backtrace->size;
#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
        char **strings = backtrace_symbols(backtrace->array, backtrace->size); 
        
        // allocate string which will be filled with the demangled function name
        size_t funcnamesize = 0;
        
        // iterate over the returned symbol lines. skip the first, it is the
        // address of this function.
        for (uint32 i = 0; i < backtrace->size; i++)
        {
            log->strings[i] = (char*)malloc(512);
            
            int len = strlen(strings[i]);
            char * temp = (char*)malloc(sizeof(char) * len + 100);
            strcpy(temp, strings[i]);

            char * tokens[100];
            int32 tokenCount = 0;
            tokens[tokenCount++] = strtok(temp," \t");
            while (tokens[tokenCount - 1] != NULL)
            {
                //Logger::Debug("%s\n",tokens[tokenCount]);
                tokens[tokenCount++] = strtok (NULL, " \t");
                if (tokenCount > 5)break;
            }
            
            if (tokenCount >= 4)
            {
                int status = -2;
                char* ret = abi::__cxa_demangle(tokens[3],
                                                NULL, &funcnamesize, &status);
                if (status == 0) 
                {
                    //funcname = ret; // use possibly realloc()-ed string
                    
                    snprintf(log->strings[i], 512, "%s(%s)",
                            strings[i], ret);
                }
                else {
                    snprintf(log->strings[i], 512, "%s(%s)", strings[i], tokens[3]);
                }
                
                free(ret);
            }else
            {
                snprintf(log->strings[i], 512, "%s", strings[i]);
            }
            free(temp);
        }
        free(strings);
#elif defined(__DAVAENGINE_WIN32__)
        
        unsigned int   i;
        void         * stack[ 100 ];
        unsigned short frames;
        SYMBOL_INFO  * symbol;
        HANDLE         process;
        
        process = GetCurrentProcess();
        
        SymInitialize( process, NULL, TRUE );
        
        symbol               = ( SYMBOL_INFO * )calloc( sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
        symbol->MaxNameLen   = 255;
        symbol->SizeOfStruct = sizeof( SYMBOL_INFO );
        
        for( i = 0; i < frames; i++ )
        {
            SymFromAddr( process, (DWORD64)( backtrace->array[i] ), 0, symbol );
            
            //printf( "%i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
        }
        
        free( symbol );
#endif
    }
    
    void ReleaseBacktraceLog(BacktraceLog * log)
    {
        for (uint32 k = 0; k < log->size; ++k)
            free(log->strings[k]);
        free(log->strings);
    }
    
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