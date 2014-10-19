
if( MACOS )
    set( ERASE_FILES	MailComposeDelegate.h 
    					MailComposeDelegate.mm )

elseif( IOS )
    set( ERASE_FILES )
    
endif()

define_source_files (PARENT_SCOPE GLOB_ERASE_FILES ${ERASE_FILES} )
