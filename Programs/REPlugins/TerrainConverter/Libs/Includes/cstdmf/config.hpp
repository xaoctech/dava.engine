#ifndef CONFIG_HPP
#define CONFIG_HPP

//#define BUILT_BY_BIGWORLD

#if defined( MF_SERVER ) && defined( CONSUMER_CLIENT )
#	error "The CONSUMER_CLIENT macro should not be used when building the server."
#endif

/**
 * This define is used to control the conditional compilation of features 
 * that will be removed from the client builds provided to the public.
 * Examples of these would be the in-game Python console and the Watcher 
 * Nub interface.
 *
 * If CONSUMER_CLIENT_BUILD is equal to zero then the array of development features should be compiled in.
 * If CONSUMER_CLIENT_BUILD is equal to one then development and maintenance only features should be excluded.
 */
#ifdef CONSUMER_CLIENT
#	define CONSUMER_CLIENT_BUILD						1
#else
#	define CONSUMER_CLIENT_BUILD						0
#endif


/**
 * These settings enable have to be manually turned on or off (no matter whether we're
 * compiling the consumer client build or not).
 */
#define ENABLE_RESOURCE_COUNTERS					(!CONSUMER_CLIENT_BUILD && !MF_SERVER)
#define ENABLE_CULLING_HUD							0


/**
 * ENABLE_MEMTRACKER turns on all memory allocation debugging features.
 */
#ifdef ENABLE_PROTECTED_ALLOCATOR
#define PROTECTED_ALLOCATOR
#elif defined ( ENABLE_MEMTRACKER )
#define FORCE_ENABLE_SLOT_TRACKER 1
#define FORCE_ENABLE_MEMORY_DEBUG 1
#define FORCE_ENABLE_ALLOCATOR_STATISTICS 1
#endif


/**
 * By setting one of the following FORCE_ENABLE_ defines to one then support for the
 * corresponding feature will be compiled in even on a consumer client build.
 */
//p_busko: want to pass FORCE_ENABLE_MSG_LOGGING as compiler option
#ifdef CONSUMER_CLIENT_STATIC
//#define FORCE_ENABLE_MSG_LOGGING					1
#define FORCE_ENABLE_DPRINTF						1
#else//CONSUMER_CLIENT_STATIC
//#define FORCE_ENABLE_MSG_LOGGING					0
#define FORCE_ENABLE_DPRINTF						0
#endif//CONSUMER_CLIENT_STATIC

#define FORCE_ENABLE_CONSOLES						0
#define FORCE_ENABLE_PYTHON_TELNET_SERVICE			0
#define FORCE_ENABLE_WATCHERS						0
#define FORCE_ENABLE_DOG_WATCHERS					0
#define FORCE_ENABLE_PROFILER						0
#define FORCE_ENABLE_HITCH_DETECTION				0
#define FORCE_ENABLE_GPU_PROFILER					0
#define FORCE_ENABLE_ACTION_QUEUE_DEBUGGER			0
#define FORCE_ENABLE_DRAW_PORTALS					0
#define FORCE_ENABLE_DRAW_SKELETON					0
#define FORCE_ENABLE_CULLING_HUD					0
#define FORCE_ENABLE_DOC_STRINGS					0
#define FORCE_ENABLE_DDS_GENERATION					0
#define FORCE_ENABLED_ASSET_PIPE					0
#define FORCE_ENABLE_FILE_CASE_CHECKING				0
#define FORCE_ENABLE_ENVIRONMENT_SYNC				0
#define FORCE_ENABLE_ENTER_DEBUGGER_MESSAGE			0
#define FORCE_ENABLE_MINI_DUMP						0
#define FORCE_ENABLE_NVIDIA_PERFHUD					0
#define FORCE_ENABLE_STACK_TRACKER					0
#define FORCE_ENABLE_WG_NVAPI_WRAPPER				0
#define FORCE_ENABLE_RELOAD_MODEL					0
#define FORCE_ENABLE_UNENCRYPTED_LOGINS				0
#define FORCE_ENABLE_SMARTPOINTER_TRACKING			0
#define FORCE_ENABLE_TRANSFORM_VALIDATION			0
#define FORCE_ENABLE_DEBUG_MESSAGE_FILE_LOG			0
#define FORCE_ENABLE_REFERENCE_COUNT_THREADING_DEBUG 0
#define FORCE_ENABLE_UNIPROF						0

#if defined(MF_SERVER)
#define DEFINED_MF_SERVER 1
#else 
#define DEFINED_MF_SERVER 0
#endif

#if defined(BW_CLIENT)
#define DEFINED_BW_CLIENT 1
#else 
#define DEFINED_BW_CLIENT 0
#endif

#if defined(_DEBUG)
#define DEFINED__DEBUG 1
#else 
#define DEFINED__DEBUG 0
#endif

#if defined(_WIN32)
#define DEFINED__WIN32 1
#else 
#define DEFINED__WIN32 0
#endif

#if defined(WINRT)
#define DEFINED_WINRT 1
#else 
#define DEFINED_WINRT 0
#endif

///
#define ENABLE_CONSOLES					(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_CONSOLES)
#define ENABLE_MSG_LOGGING				(!CONSUMER_CLIENT_BUILD || DEFINED_MF_SERVER || BOTS || EDITOR_ENABLED || FORCE_ENABLE_MSG_LOGGING || BW_EMBEDDED)

// enable for log into Output window
#define ENABLE_DPRINTF					(!BW_CLIENT || FORCE_ENABLE_DPRINTF || BW_EMBEDDED)

#define ENABLE_PYTHON_TELNET_SERVICE	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_PYTHON_TELNET_SERVICE)
#define ENABLE_WATCHERS					(!BW_EXPORTER && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_WATCHERS))
#define ENABLE_DOG_WATCHERS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DOG_WATCHERS)
#if 0
#define ENABLE_PROFILER					(!defined( __APPLE__ ) && !defined( __ANDROID__ ) && \
											(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_PROFILER))
#else
#define ENABLE_PROFILER					(DEFINED_MF_SERVER || EDITOR_ENABLED || FORCE_ENABLE_PROFILER) //WG disable
#endif

#define ENABLE_HITCH_DETECTION			(!DEFINED_MF_SERVER && ENABLE_PROFILER && (FORCE_ENABLE_HITCH_DETECTION))
#define ENABLE_GPU_PROFILER				(ENABLE_PROFILER && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_GPU_PROFILER))
#define ENABLE_PER_CORE_PROFILER		( 0 )
#define ENABLE_NVIDIA_PERFKIT           (ENABLE_GPU_PROFILER)
#define ENABLE_ACTION_QUEUE_DEBUGGER	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ACTION_QUEUE_DEBUGGER)
#define ENABLE_DRAW_PORTALS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DRAW_PORTALS)
#define ENABLE_DRAW_SKELETON			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DRAW_SKELETON)
//#define ENABLE_CULLING_HUD				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_CULLING_HUD)
#define ENABLE_DOC_STRINGS				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DOC_STRINGS)
#define ENABLE_DDS_GENERATION			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_DDS_GENERATION)
#define ENABLE_ASSET_PIPE				(defined( EDITOR_ENABLED ) || defined( _NAVGEN ) ||\
											(DEFINED_BW_CLIENT && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLED_ASSET_PIPE)))
#define ENABLE_FILE_CASE_CHECKING		(!BW_EXPORTER && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_FILE_CASE_CHECKING))
#define ENABLE_ENVIRONMENT_SYNC			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ENVIRONMENT_SYNC)
#define ENABLE_ENTER_DEBUGGER_MESSAGE	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_ENTER_DEBUGGER_MESSAGE)
#define ENABLE_MINI_DUMP				(DEFINED__WIN32 && !DEFINED_WINRT && (!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_MINI_DUMP))
#define ENABLE_NVIDIA_PERFHUD			(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_NVIDIA_PERFHUD)
#if 0 || defined(BOTS)
#define ENABLE_MEMORY_DEBUG				(!BWCLIENT_AS_PYTHON_MODULE && !BW_EXPORTER && \
											((!defined(MF_SERVER) && !CONSUMER_CLIENT_BUILD && !EDITOR_ENABLED) || FORCE_ENABLE_MEMORY_DEBUG))
#else
#define ENABLE_MEMORY_DEBUG				0 // WG disabled
#endif
#define ENABLE_SMARTPOINTER_TRACKING	(ENABLE_MEMORY_DEBUG && FORCE_ENABLE_SMARTPOINTER_TRACKING)

#define ENABLE_SMARTPOINTER_TRACKING	(ENABLE_MEMORY_DEBUG && FORCE_ENABLE_SMARTPOINTER_TRACKING)
#define ENABLE_STACK_TRACKER			(!BW_EXPORTER && (!CONSUMER_CLIENT_BUILD || ENABLE_MEMORY_DEBUG || FORCE_ENABLE_STACK_TRACKER) && \
											!(DEFINED__WIN32 && defined( __clang__ )))
#define ENABLE_ALLOCATOR_STATISTICS		ENABLE_MEMORY_DEBUG && (!BWCLIENT_AS_PYTHON_MODULE && !BW_EXPORTER && \
											((!DEFINED_MF_SERVER && !CONSUMER_CLIENT_BUILD && !EDITOR_ENABLED) || FORCE_ENABLE_ALLOCATOR_STATISTICS))
#define ENABLE_SLOT_TRACKER				(ENABLE_MEMORY_DEBUG || ENABLE_ALLOCATOR_STATISTICS)
#define ENABLE_RELOAD_MODEL				(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_RELOAD_MODEL) && !DEFINED_MF_SERVER
#define ENABLE_FIXED_SIZED_POOL_ALLOCATOR (DEFINED__WIN32 || (DEFINED_MF_SERVER && ENABLE_MEMORY_DEBUG))
#define ENABLE_FIXED_SIZED_POOL_STATISTICS (!CONSUMER_CLIENT_BUILD && ENABLE_FIXED_SIZED_POOL_ALLOCATOR && !DEFINED_MF_SERVER)
#define ENABLE_UNENCRYPTED_LOGINS		(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_UNENCRYPTED_LOGINS)
#define ENABLE_REFERENCE_COUNT_THREADING_DEBUG (DEFINED__DEBUG || FORCE_ENABLE_REFERENCE_COUNT_THREADING_DEBUG)


#define ENABLE_WG_NVAPI_WRAPPER			(BW_CLIENT || FORCE_ENABLE_WG_NVAPI_WRAPPER)
#define ENABLE_WG_RENDER_PERF_MARKERS	(!CONSUMER_CLIENT_BUILD && !BW_EXPORTER && !MF_SERVER && !EDITOR_ENABLED && !_NAVGEN)

#define ENABLE_UNIPROF					((!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_UNIPROF) && !DEFINED_MF_SERVER && !BW_EXPORTER && !EDITOR_ENABLED && !_NAVGEN && !IS_RES_PACKER && !IS_ASSETPROCESSOR)


//--------------------------------------------------------------------------------------------------
//-- WG Profiler

// Set to 1 to enable realtime function-level profiling (press ~ + F9 to show results in realtime).
#define WG_PROFILER_ENABLE			0

// If set to 1, WGScopeProfiler::enableProfiler(true) will be added in beginning of the BWWinMain 
// function.
#define WG_PROFILER_AUTORUN			0

// WG - If set to 1, WG_PROFILER macro will be added to all BW_GUARD macroes; otherwise WG_PROFILER
#define WG_PROFILER_COVER_ALL		0

//--------------------------------------------------------------------------------------------------

// WG - enable or disable optional BugTrap-based error reporting (requires BugTrapU.dll in game directory).
// If enabled and dll is found, this will work in both Hybrid and Consumer versions.
#if defined(EDITOR_ENABLED) || defined(MF_SERVER) || defined(PROCESS_DEFS) || defined(BOTS) \
	|| defined(IS_RES_PACKER) || defined(IS_ASSETPROCESSOR) || defined(BW_EXPORTER)
	#define WG_ENABLE_ERROR_REPORT 0
#else
	#define WG_ENABLE_ERROR_REPORT 1
#endif

#if defined(EDITOR_ENABLED)
	#define WG_ENABLE_CRASH_REPORTER
#endif

// WG - steam integration
#if defined(EDITOR_ENABLED) || defined(_NAVGEN) || defined(PROCESS_DEFS) \
	|| defined(MF_SERVER) || defined(BW_EXPORTER)
	#define STEAM_ENABLED 0
#else
	#define STEAM_ENABLED 1
#endif

#if CONSUMER_CLIENT_BUILD
	#define WG_SINGLE_APP_INSTANCE
#endif

// Disabled in tools because they have random events which can switch models
// in between update and draw
#define ENABLE_TRANSFORM_VALIDATION		DEFINED_BW_CLIENT && \
 	(!CONSUMER_CLIENT_BUILD || FORCE_ENABLE_TRANSFORM_VALIDATION)

/**
 *	Target specific restrictions.
 */
#if defined( PLAYSTATION3 )
#	undef  ENABLE_WATCHERS
#	define ENABLE_WATCHERS				0
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( _XBOX360 )
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( __APPLE__ ) || defined( __ANDROID__ ) || defined (WINRT)
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#	undef  ENABLE_ALLOCATOR_STATISTICS
#	define ENABLE_ALLOCATOR_STATISTICS	0
#	undef ENABLE_SLOT_TRACKER
#	define ENABLE_SLOT_TRACKER 0
#endif

#if defined( EMSCRIPTEN )
#	undef  ENABLE_WATCHERS
#	define ENABLE_WATCHERS				0
#	undef  ENABLE_PROFILER
#	define ENABLE_PROFILER				0
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

#if defined( BWCLIENT_AS_PYTHON_MODULE )
#	undef  ENABLE_STACK_TRACKER
#	define ENABLE_STACK_TRACKER			0
#endif

// Undef anything that was relying on stack tracker
#if !ENABLE_STACK_TRACKER
#	undef ENABLE_MEMORY_DEBUG
#	define ENABLE_MEMORY_DEBUG 0
#endif

#endif // CONFIG_HPP
