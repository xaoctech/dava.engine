
if( APPLE AND NOT IOS )
    set( ERASE_FILES  MailComposeDelegate.h MailComposeDelegate.mm )

elseif( IOS )
    set( ERASE_FILES )
    
endif()

define_source_files (
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)