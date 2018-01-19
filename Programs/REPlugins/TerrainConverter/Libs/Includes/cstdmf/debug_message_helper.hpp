#ifndef DEBUG_MESSAGE_HELPER_HPP
#define DEBUG_MESSAGE_HELPER_HPP

#include "bw_string.hpp"
#include "debug_message_priority.hpp"
#include "debug_message_source.hpp"
#include "stdmf.hpp"
#include <stdarg.h>

BW_BEGIN_NAMESPACE

class SimpleMutex;

/**
 *  This class implements the functionality exposed by BigWorld message macros,
 *	manages calling registered message callbacks, and handles both critical
 *  and non-critical messages.
 */
class DebugMessageHelper
{
public:
	CSTDMF_DLL DebugMessageHelper( DebugMessagePriority componentPriority,
			DebugMessagePriority messagePriority,
			const char * category = "",
			DebugMessageSource messageSource = MESSAGE_SOURCE_CPP,
			const char * pMetaData = NULL );

	CSTDMF_DLL DebugMessageHelper();

	CSTDMF_DLL static void fini();

#if defined( __GNUC__ ) || defined( __clang__ )
	CSTDMF_DLL void message( const char * pFormat, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
	CSTDMF_DLL void messageWithMetaData( const char * pMetaData,
			const char * pFormat, ... )
        __attribute__ ( (format (printf, 3, 4 ) ) );
    CSTDMF_DLL void criticalMessage( const char * pFormat, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
    CSTDMF_DLL void devCriticalMessage( const char * pFormat, ... )
        __attribute__ ( (format (printf, 2, 3 ) ) );
#else // defined( __GNUC__ ) || defined( __clang__ )
	CSTDMF_DLL void message( const char * pFormat, ... );
	CSTDMF_DLL void messageWithMetaData( const char * pMetaData,
		const char * pFormat, ... );
	CSTDMF_DLL void criticalMessage( const char * pFormat, ... );
	CSTDMF_DLL void devCriticalMessage( const char * pFormat, ... );
#endif // defined( __GNUC__ ) || defined( __clang__ )

	CSTDMF_DLL void messageBackTrace();
	static void shouldWriteToSyslog( bool state = true );

	CSTDMF_DLL static void formatMessage( char * /*out*/pMsgBuffer,
		size_t lenMsgBuffer, const char * pFormat, va_list argPtr );

	static void criticalMsgOccurs( bool occurs )
		{	criticalMsgOccurs_ = occurs;	}
	static bool criticalMsgOccurs()
		{	return criticalMsgOccurs_;	}

#ifdef _WIN32
	CSTDMF_DLL static void logToFile( const char* line );
	CSTDMF_DLL static void automatedTest( bool isTest, const char* debugLogPath = NULL)
    {
        automatedTest_ = isTest;
        if ( NULL != debugLogPath)
            bw_sprintf(debugLogFile_, sizeof( debugLogFile_ ), "%s\\debug.log", debugLogPath);
    }
	CSTDMF_DLL static bool automatedTest() { return automatedTest_; }
#endif // _WIN32

private:
	void messageHelper( const char * pFormat, va_list argPtr, 
			const char * pMetaData );
	void criticalMessageHelper( bool isDevAssertion, const char * pFormat,
			va_list argPtr );

	bool shouldAcceptMessage();

	DebugMessagePriority componentPriority_;
	DebugMessagePriority messagePriority_;
	const char * category_;
	DebugMessageSource messageSource_;
	const char * pMetaData_;

	static bool criticalMsgOccurs_;
#ifdef _WIN32
	static bool automatedTest_;
	static char debugLogFile_[1024];
#endif // _WIN32
};

BW_END_NAMESPACE

#endif // DEBUG_MESSAGE_HELPER_HPP
