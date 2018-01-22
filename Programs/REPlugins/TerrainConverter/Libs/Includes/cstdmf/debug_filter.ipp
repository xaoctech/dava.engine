#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 *	This method returns the singleton instance of this class.
 */
INLINE /*static*/ DebugFilter & DebugFilter::instance()
{
	if (s_instance_ == NULL)
	{
		s_instance_ = new DebugFilter();
	}

	return *s_instance_;
}


/**
 *	
 */
INLINE /*static*/ void DebugFilter::fini()
{
	if (s_instance_)
	{
		bw_safe_delete( s_instance_ );
	}
}


/**
 *	This method returns whether or not a message with the input priorities
 *	should be accepted.
 */
INLINE /*static*/ bool DebugFilter::shouldAccept(
	DebugMessagePriority componentPriority,
	DebugMessagePriority messagePriority, const BW::string & categoryName )
{
	if (messagePriority >=
		std::max(
			(DebugMessagePriority)DebugFilter::instance().filterThreshold(),
			componentPriority ))
	{
		return DebugFilter::instance().shouldAcceptCategory( categoryName,
				messagePriority );
	}

	return false;
}


/**
 *	
 */
INLINE /*static*/ bool DebugFilter::shouldWriteTimePrefix()
{
	return s_shouldWriteTimePrefix_;
}


/**
 *	
 */
INLINE /*static*/ void DebugFilter::shouldWriteTimePrefix( bool value )
{
	s_shouldWriteTimePrefix_ = value;
}


/**
 *	
 */
INLINE /*static*/ bool DebugFilter::shouldWriteToConsole()
{
	return s_shouldWriteToConsole_;
}


/**
 *	
 */
INLINE /*static*/ void DebugFilter::shouldWriteToConsole( bool value )
{
	s_shouldWriteToConsole_ = value;
}


/**
 *	
 */
INLINE /*static*/ bool DebugFilter::shouldOutputErrorBackTrace()
{
	return s_shouldOutputErrorBackTrace_;
}


/**
 *	
 */
INLINE /*static*/ void DebugFilter::shouldOutputErrorBackTrace( bool value )
{
	s_shouldOutputErrorBackTrace_ = value;
}


/**
 *	
 */
INLINE /*static*/ void DebugFilter::consoleOutputFile( FILE * value )
{
	s_consoleOutputFile_.set( value );
}


/**
 *	
 */
INLINE /*static*/ FILE * DebugFilter::consoleOutputFile()
{
	return s_consoleOutputFile_.get();
}

// debug_filter.ipp
